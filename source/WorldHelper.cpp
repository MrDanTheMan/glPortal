#include <glPortal/WorldHelper.hpp>
#include <glPortal/Portal.hpp>

#include <bullet/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>

#include <radix/data/texture/TextureLoader.hpp>
#include <radix/entities/traits/MeshDrawableTrait.hpp>
#include <radix/simulation/Physics.hpp>
#include <radix/entities/traits/LightSourceTrait.hpp>
#include <radix/util/BulletUserPtrInfo.hpp>

using namespace radix;

namespace glPortal {

void WorldHelper::shootPortal(int button, World &world) {
  Vector3f cameraDir = Math::toDirection(world.camera->getOrientation());
  btVector3 btFrom = world.camera->getPosition();
  btVector3 btTo = btFrom + cameraDir*10000;
  btCollisionWorld::ClosestRayResultCallback res(btFrom, btTo);

  simulation::Physics &phys = world.simulations.findFirstOfType<simulation::Physics>();
  phys.getPhysicsWorld().rayTest(btFrom, btTo, res);

  if (res.hasHit()) {
    const Entity *pEnt = util::getBtPtrInfo(res.m_collisionObject).entity;
    // All RigidBodies should have their pointer set, but check anyway
    if (pEnt) {
      const Entity &ent = *pEnt;
      // TODO: material in separate Component, + 1 mat per face
      const auto *mdt = dynamic_cast<const entities::MeshDrawableTrait*>(&ent);
      if (mdt and
          mdt->material.portalable) {
        EntityPair &pPair = getPortalPair(0, world);
        Vector3f ipos(res.m_hitPointWorld);
        Entity &pEnt = (button == 1) ? *pPair.first : *pPair.second;
        Portal &portal = dynamic_cast<Portal&>(pEnt);
        portal.openSince = world.getTime();
        portal.maskTex.diffuse = TextureLoader::getTexture("portalmask.png");
        portal.placeOnWall(world.camera->getPosition(), ipos, res.m_hitNormalWorld);
        auto &pLight = static_cast<entities::LightSourceTrait&>(portal);

        if (button == 1) {
          portal.overlayTex.diffuse = TextureLoader::getTexture("blueportal.png");
          portal.color = pLight.color = Portal::BLUE_COLOR;
        } else {
          portal.overlayTex.diffuse = TextureLoader::getTexture("orangeportal.png");
          portal.color = pLight.color = Portal::ORANGE_COLOR;
        }
      }
    }
  }
}

EntityPair& WorldHelper::getPortalPair(int pair, World &world) {
  return world.entityPairs.at("portalPairs").at(pair);
}

void WorldHelper::closePortals(World &world) {
  EntityPair &pPair = getPortalPair(0, world);

  dynamic_cast<Portal*>(pPair.first)->open = false;
  dynamic_cast<Portal*>(pPair.second)->open = false;
}

} /* namespace glPortal */
