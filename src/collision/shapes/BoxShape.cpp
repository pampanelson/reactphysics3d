/********************************************************************************
* ReactPhysics3D physics library, http://code.google.com/p/reactphysics3d/      *
* Copyright (c) 2010-2013 Daniel Chappuis                                       *
*********************************************************************************
*                                                                               *
* This software is provided 'as-is', without any express or implied warranty.   *
* In no event will the authors be held liable for any damages arising from the  *
* use of this software.                                                         *
*                                                                               *
* Permission is granted to anyone to use this software for any purpose,         *
* including commercial applications, and to alter it and redistribute it        *
* freely, subject to the following restrictions:                                *
*                                                                               *
* 1. The origin of this software must not be misrepresented; you must not claim *
*    that you wrote the original software. If you use this software in a        *
*    product, an acknowledgment in the product documentation would be           *
*    appreciated but is not required.                                           *
*                                                                               *
* 2. Altered source versions must be plainly marked as such, and must not be    *
*    misrepresented as being the original software.                             *
*                                                                               *
* 3. This notice may not be removed or altered from any source distribution.    *
*                                                                               *
********************************************************************************/

// Libraries
#include "BoxShape.h"
#include "collision/ProxyShape.h"
#include "configuration.h"
#include <vector>
#include <cassert>

using namespace reactphysics3d;

// Constructor
BoxShape::BoxShape(const Vector3& extent, decimal margin)
         : CollisionShape(BOX, margin), mExtent(extent - Vector3(margin, margin, margin)) {
    assert(extent.x > decimal(0.0) && extent.x > margin);
    assert(extent.y > decimal(0.0) && extent.y > margin);
    assert(extent.z > decimal(0.0) && extent.z > margin);
}

// Private copy-constructor
BoxShape::BoxShape(const BoxShape& shape) : CollisionShape(shape), mExtent(shape.mExtent) {

}

// Destructor
BoxShape::~BoxShape() {

}

// Return the local inertia tensor of the collision shape
void BoxShape::computeLocalInertiaTensor(Matrix3x3& tensor, decimal mass) const {
    decimal factor = (decimal(1.0) / decimal(3.0)) * mass;
    Vector3 realExtent = mExtent + Vector3(mMargin, mMargin, mMargin);
    decimal xSquare = realExtent.x * realExtent.x;
    decimal ySquare = realExtent.y * realExtent.y;
    decimal zSquare = realExtent.z * realExtent.z;
    tensor.setAllValues(factor * (ySquare + zSquare), 0.0, 0.0,
                        0.0, factor * (xSquare + zSquare), 0.0,
                        0.0, 0.0, factor * (xSquare + ySquare));
}

// Raycast method
bool BoxShape::raycast(const Ray& ray, ProxyShape* proxyShape) const {

    const Transform localToWorldTransform = proxyShape->getLocalToWorldTransform();
    const Transform worldToLocalTransform = localToWorldTransform.getInverse();
    Vector3 origin = worldToLocalTransform * ray.origin;
    Vector3 rayDirection = worldToLocalTransform.getOrientation() * ray.direction.getUnit();
    decimal tMin = decimal(0.0);
    decimal tMax = DECIMAL_LARGEST;

    // For each of the three slabs
    for (int i=0; i<3; i++) {

        // If ray is parallel to the slab
        if (std::abs(rayDirection[i]) < MACHINE_EPSILON) {

            // If the ray's origin is not inside the slab, there is no hit
            if (origin[i] > mExtent[i] || origin[i] < -mExtent[i]) return false;
        }
        else {

            // Compute the intersection of the ray with the near and far plane of the slab
            decimal oneOverD = decimal(1.0) / rayDirection[i];
            decimal t1 = (-mExtent[i] - origin[i]) * oneOverD;
            decimal t2 = (mExtent[i] - origin [i]) * oneOverD;

            // Swap t1 and t2 if need so that t1 is intersection with near plane and
            // t2 with far plane
            if (t1 > t2) swap(t1, t2);

            // If t1 is negative, the origin is inside the box and therefore, there is no hit
            if (t1 < decimal(0.0)) return false;

            // Compute the intersection of the of slab intersection interval with previous slabs
            tMin = std::max(tMin, t1);
            tMax = std::min(tMax, t2);

            // If the slabs intersection is empty, there is no hit
            if (tMin > tMax) return false;
        }
    }

    // A hit has been found
    return true;
}

// Raycast method with feedback information
bool BoxShape::raycast(const Ray& ray, RaycastInfo& raycastInfo, ProxyShape* proxyShape,
                       decimal distance) const {

    const Transform localToWorldTransform = proxyShape->getLocalToWorldTransform();
    const Transform worldToLocalTransform = localToWorldTransform.getInverse();
    Vector3 origin = worldToLocalTransform * ray.origin;
    Vector3 rayDirection = worldToLocalTransform.getOrientation() * ray.direction.getUnit();
    decimal tMin = decimal(0.0);
    decimal tMax = DECIMAL_LARGEST;
    Vector3 normalDirection(decimal(0), decimal(0), decimal(0));
    Vector3 currentNormal;

    // For each of the three slabs
    for (int i=0; i<3; i++) {

        // If ray is parallel to the slab
        if (std::abs(rayDirection[i]) < MACHINE_EPSILON) {

            // If the ray's origin is not inside the slab, there is no hit
            if (origin[i] > mExtent[i] || origin[i] < -mExtent[i]) return false;
        }
        else {

            // Compute the intersection of the ray with the near and far plane of the slab
            decimal oneOverD = decimal(1.0) / rayDirection[i];
            decimal t1 = (-mExtent[i] - origin[i]) * oneOverD;
            decimal t2 = (mExtent[i] - origin [i]) * oneOverD;
            currentNormal = -mExtent;

            // Swap t1 and t2 if need so that t1 is intersection with near plane and
            // t2 with far plane
            if (t1 > t2) {
                swap(t1, t2);
                currentNormal = -currentNormal;
            }

            // If t1 is negative, the origin is inside the box and therefore, there is no hit
            if (t1 < decimal(0.0)) return false;

            // Compute the intersection of the of slab intersection interval with previous slabs
            if (t1 > tMin) {
                tMin = t1;
                normalDirection = currentNormal;
            }
            tMax = std::min(tMax, t2);

            // If tMin is larger than the maximum raycasting distance, we return no hit
            if (tMin > distance) return false;

            // If the slabs intersection is empty, there is no hit
            if (tMin > tMax) return false;
        }
    }

    // The ray intersects the three slabs, we compute the hit point
    Vector3 localHitPoint = origin + tMin * rayDirection;

    raycastInfo.body = proxyShape->getBody();
    raycastInfo.proxyShape = proxyShape;
    raycastInfo.distance = tMin;
    raycastInfo.worldPoint = localToWorldTransform * localHitPoint;
    raycastInfo.worldNormal = localToWorldTransform.getOrientation() * normalDirection;

    return true;
}
