/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <box2dtools.hxx>

#include <shapemanager.hxx>
#include <attributableshape.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygontriangulator.hxx>

#include <svx/svdobj.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svdpage.hxx>

#include <svx/unoapi.hxx>
#include <utility>

#define BOX2D_SLIDE_SIZE_IN_METERS 100.00f
constexpr float fDefaultStaticBodyBounciness(0.1f);
constexpr float fDefaultStaticDensity(1.0f);
constexpr float fDefaultStaticFriction(0.3f);

namespace box2d::utils
{
namespace
{
double calculateScaleFactor(const ::basegfx::B2DVector& rSlideSize)
{
    double fWidth = rSlideSize.getX();
    double fHeight = rSlideSize.getY();

    // Scale factor is based on whatever is the larger
    // value between slide width and height
    if (fWidth > fHeight)
        return BOX2D_SLIDE_SIZE_IN_METERS / fWidth;
    else
        return BOX2D_SLIDE_SIZE_IN_METERS / fHeight;
}

b2BodyType getBox2DInternalBodyType(const box2DBodyType eType)
{
    switch (eType)
    {
        default:
        case BOX2D_STATIC_BODY:
            return b2_staticBody;
        case BOX2D_KINEMATIC_BODY:
            return b2_kinematicBody;
        case BOX2D_DYNAMIC_BODY:
            return b2_dynamicBody;
    }
}

box2DBodyType getBox2DLOBodyType(const b2BodyType eType)
{
    switch (eType)
    {
        default:
        case b2_staticBody:
            return BOX2D_STATIC_BODY;
        case b2_kinematicBody:
            return BOX2D_KINEMATIC_BODY;
        case b2_dynamicBody:
            return BOX2D_DYNAMIC_BODY;
    }
}

b2Vec2 convertB2DPointToBox2DVec2(const basegfx::B2DPoint& aPoint, const double fScaleFactor)
{
    return { static_cast<float>(aPoint.getX() * fScaleFactor),
             static_cast<float>(aPoint.getY() * -fScaleFactor) };
}

// expects rTriangleVector to have coordinates relative to the shape's bounding box center
void addTriangleVectorToBody(const basegfx::triangulator::B2DTriangleVector& rTriangleVector,
#if BOX2D_CHECK_VERSION(3, 0)
                             b2BodyId* aBody, const double fScaleFactor)
#else
                             b2Body* aBody, const double fScaleFactor)
#endif
{
    for (const basegfx::triangulator::B2DTriangle& aTriangle : rTriangleVector)
    {
        b2Vec2 aTriangleVertices[3]
            = { convertB2DPointToBox2DVec2(aTriangle.getA(), fScaleFactor),
                convertB2DPointToBox2DVec2(aTriangle.getB(), fScaleFactor),
                convertB2DPointToBox2DVec2(aTriangle.getC(), fScaleFactor) };

        bool bValidPointDistance
            = b2DistanceSquared(aTriangleVertices[0], aTriangleVertices[1]) > 0.003f
              && b2DistanceSquared(aTriangleVertices[0], aTriangleVertices[2]) > 0.003f
              && b2DistanceSquared(aTriangleVertices[1], aTriangleVertices[2]) > 0.003f;

        if (bValidPointDistance)
        {
        // create a fixture that represents the triangle
#if BOX2D_CHECK_VERSION(3, 0)
            b2Hull aHull = b2ComputeHull(aTriangleVertices, 3);
            b2ShapeDef aShapeDef = b2DefaultShapeDef();
            aShapeDef.density = fDefaultStaticDensity;
#if BOX2D_CHECK_VERSION(3, 1)
            aShapeDef.material.friction = fDefaultStaticFriction;
            aShapeDef.material.restitution = fDefaultStaticBodyBounciness;
#else
            aShapeDef.friction = fDefaultStaticFriction;
            aShapeDef.restitution = fDefaultStaticBodyBounciness;
#endif
            b2Polygon aPolygonShape = b2MakePolygon(&aHull, 0.0);
            b2CreatePolygonShape(*aBody, &aShapeDef, &aPolygonShape);
#else
            b2FixtureDef aFixture;
            b2PolygonShape aPolygonShape;
            aPolygonShape.Set(aTriangleVertices, 3);
            aFixture.shape = &aPolygonShape;
            aFixture.density = fDefaultStaticDensity;
            aFixture.friction = fDefaultStaticFriction;
            aFixture.restitution = fDefaultStaticBodyBounciness;
            aBody->CreateFixture(&aFixture);
#endif
        }
    }
}

// expects rPolygon to have coordinates relative to it's center
#if BOX2D_CHECK_VERSION(3, 0)
void addEdgeShapeToBody(const basegfx::B2DPolygon& rPolygon, b2BodyId* aBody,
#else
void addEdgeShapeToBody(const basegfx::B2DPolygon& rPolygon, b2Body* aBody,
#endif
                        const double fScaleFactor)
{
    // make sure there's no bezier curves on the polygon
    assert(!rPolygon.areControlPointsUsed());
    basegfx::B2DPolygon aPolygon = basegfx::utils::removeNeutralPoints(rPolygon);

    // value that somewhat defines half width of the quadrilateral
    // that will be representing edge segment in the box2d world
    const float fHalfWidth = 0.1f;
    bool bHasPreviousQuadrilateralEdge = false;
    b2Vec2 aQuadrilateralVertices[4];

    for (sal_uInt32 nIndex = 0; nIndex < aPolygon.count(); nIndex++)
    {
        basegfx::B2DPoint aPointA;
        basegfx::B2DPoint aPointB;
        if (nIndex != 0)
        {
            // get two adjacent points to create an edge out of
            aPointA = aPolygon.getB2DPoint(nIndex - 1);
            aPointB = aPolygon.getB2DPoint(nIndex);
        }
        else if (aPolygon.isClosed())
        {
            // start by connecting the last point to the first one
            aPointA = aPolygon.getB2DPoint(aPolygon.count() - 1);
            aPointB = aPolygon.getB2DPoint(nIndex);
        }
        else // the polygon isn't closed, won't connect last and first points
        {
            continue;
        }

        // create a vector that represents the direction of the edge
        // and make it a unit vector
        b2Vec2 aEdgeUnitVec(convertB2DPointToBox2DVec2(aPointB, fScaleFactor)
                            - convertB2DPointToBox2DVec2(aPointA, fScaleFactor));
#if BOX2D_CHECK_VERSION(3, 0)
        aEdgeUnitVec = b2Normalize(aEdgeUnitVec);
#else
        aEdgeUnitVec.Normalize();
#endif

        // create a unit vector that represents Normal of the edge
#if BOX2D_CHECK_VERSION(3, 0)
        b2Vec2 aEdgeNormal = { -aEdgeUnitVec.y, aEdgeUnitVec.x };
#else
        b2Vec2 aEdgeNormal(-aEdgeUnitVec.y, aEdgeUnitVec.x);
#endif

        // if there was an edge previously created it should just connect
        // using it's ending points so that there are no empty spots
        // between edge segments, if not use wherever aPointA is at
        if (!bHasPreviousQuadrilateralEdge)
        {
            // the point is translated along the edge normal both directions by
            // fHalfWidth to create a quadrilateral edge
            aQuadrilateralVertices[0]
                = convertB2DPointToBox2DVec2(aPointA, fScaleFactor) + fHalfWidth * aEdgeNormal;
            aQuadrilateralVertices[1]
                = convertB2DPointToBox2DVec2(aPointA, fScaleFactor) + -fHalfWidth * aEdgeNormal;
            bHasPreviousQuadrilateralEdge = true;
        }
        aQuadrilateralVertices[2]
            = convertB2DPointToBox2DVec2(aPointB, fScaleFactor) + fHalfWidth * aEdgeNormal;
        aQuadrilateralVertices[3]
            = convertB2DPointToBox2DVec2(aPointB, fScaleFactor) + -fHalfWidth * aEdgeNormal;

        // check whether the edge would have degenerately close points
        bool bValidPointDistance
            = b2DistanceSquared(aQuadrilateralVertices[0], aQuadrilateralVertices[2]) > 0.003f;

        if (bValidPointDistance)
        {
        // create a quadrilateral shaped fixture to represent the edge
#if BOX2D_CHECK_VERSION(3, 0)
            b2Hull aHull = b2ComputeHull(aQuadrilateralVertices, 4);
            b2ShapeDef aShapeDef = b2DefaultShapeDef();
            aShapeDef.density = fDefaultStaticDensity;
#if BOX2D_CHECK_VERSION(3, 1)
            aShapeDef.material.friction = fDefaultStaticFriction;
            aShapeDef.material.restitution = fDefaultStaticBodyBounciness;
#else
            aShapeDef.friction = fDefaultStaticFriction;
            aShapeDef.restitution = fDefaultStaticBodyBounciness;
#endif
            b2Polygon aPolygonShape = b2MakePolygon(&aHull, 0.0);
            b2CreatePolygonShape(*aBody, &aShapeDef, &aPolygonShape);
#else
            b2FixtureDef aFixture;
            b2PolygonShape aPolygonShape;
            aPolygonShape.Set(aQuadrilateralVertices, 4);
            aFixture.shape = &aPolygonShape;
            aFixture.density = fDefaultStaticDensity;
            aFixture.friction = fDefaultStaticFriction;
            aFixture.restitution = fDefaultStaticBodyBounciness;
            aBody->CreateFixture(&aFixture);
#endif

            // prepare the quadrilateral edge for next connection
            aQuadrilateralVertices[0] = aQuadrilateralVertices[2];
            aQuadrilateralVertices[1] = aQuadrilateralVertices[3];
        }
    }
}

#if BOX2D_CHECK_VERSION(3, 0)
void addEdgeShapeToBody(const basegfx::B2DPolyPolygon& rPolyPolygon, b2BodyId* aBody,
#else
void addEdgeShapeToBody(const basegfx::B2DPolyPolygon& rPolyPolygon, b2Body* aBody,
#endif
                        const double fScaleFactor)
{
    for (const basegfx::B2DPolygon& rPolygon : rPolyPolygon)
    {
        addEdgeShapeToBody(rPolygon, aBody, fScaleFactor);
    }
}
}

box2DWorld::box2DWorld(const ::basegfx::B2DVector& rSlideSize)
#if BOX2D_CHECK_VERSION(3, 0)
    : mbWorldInitialized(false)
#else
    : mpBox2DWorld()
#endif
    , mfScaleFactor(calculateScaleFactor(rSlideSize))
    , mbShapesInitialized(false)
    , mbHasWorldStepper(false)
    , mbAlreadyStepped(false)
    , mnPhysicsAnimationCounter(0)
    , mpXShapeToBodyMap()
    , maShapeParallelUpdateQueue()
{
}

box2DWorld::~box2DWorld() = default;

void box2DWorld::createStaticFrameAroundSlide(const ::basegfx::B2DVector& rSlideSize)
{
#if BOX2D_CHECK_VERSION(3, 0)
    b2Vec2 gravity = { 0.0f, -30.0f };
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = gravity;

    maBox2DWorldId = b2CreateWorld(&worldDef);
    mbWorldInitialized = true;
#else
    mpBox2DWorld = std::make_unique<b2World>(b2Vec2(0.0f, -30.0f));
#endif

    float fWidth = static_cast<float>(rSlideSize.getX() * mfScaleFactor);
    float fHeight = static_cast<float>(rSlideSize.getY() * mfScaleFactor);

    // static body for creating the frame around the slide
    b2BodyDef aBodyDef;
    aBodyDef.type = b2_staticBody;
#if BOX2D_CHECK_VERSION(3, 0)
    aBodyDef.position = { 0, 0 };
#else
    aBodyDef.position.Set(0, 0);
#endif

    // not going to be stored anywhere, will live
    // as long as the Box2DWorld does
#if BOX2D_CHECK_VERSION(3, 0)
    b2BodyId aBody = b2CreateBody(maBox2DWorldId, &aBodyDef);

    // create an edge loop that represents slide frame
    b2Vec2 aEdgePoints[4] = { { 0, 0 }, { 0, -fHeight }, { fWidth, -fHeight }, { fWidth, 0 } };

    b2ChainDef aChainDef = b2DefaultChainDef();
    aChainDef.points = aEdgePoints;
    aChainDef.count = 4;
    aChainDef.isLoop = true;
    b2CreateChain(aBody, &aChainDef);
#else
    b2Body* pStaticBody = mpBox2DWorld->CreateBody(&aBodyDef);

    // create an edge loop that represents slide frame
    b2Vec2 aEdgePoints[4];
    aEdgePoints[0].Set(0, 0);
    aEdgePoints[1].Set(0, -fHeight);
    aEdgePoints[2].Set(fWidth, -fHeight);
    aEdgePoints[3].Set(fWidth, 0);

    b2ChainShape aEdgesChainShape;
    aEdgesChainShape.CreateLoop(aEdgePoints, 4);

    // create the fixture for the shape
    b2FixtureDef aFixtureDef;
    aFixtureDef.shape = &aEdgesChainShape;
    pStaticBody->CreateFixture(&aFixtureDef);
#endif
}

void box2DWorld::setShapePosition(const css::uno::Reference<css::drawing::XShape>& xShape,
                                  const basegfx::B2DPoint& rOutPos)
{
    const auto iter = mpXShapeToBodyMap.find(xShape);
    assert(iter != mpXShapeToBodyMap.end());
    Box2DBodySharedPtr pBox2DBody = iter->second;
    pBox2DBody->setPosition(rOutPos);
}

void box2DWorld::setShapePositionByLinearVelocity(
    const css::uno::Reference<css::drawing::XShape>& xShape, const basegfx::B2DPoint& rOutPos,
    const double fPassedTime)
{
    if (fPassedTime > 0) // this only makes sense if there was an advance in time
    {
        const auto iter = mpXShapeToBodyMap.find(xShape);
        assert(iter != mpXShapeToBodyMap.end());
        Box2DBodySharedPtr pBox2DBody = iter->second;
        pBox2DBody->setPositionByLinearVelocity(rOutPos, fPassedTime);
    }
}

void box2DWorld::setShapeLinearVelocity(const css::uno::Reference<css::drawing::XShape>& xShape,
                                        const basegfx::B2DVector& rVelocity)
{
    const auto iter = mpXShapeToBodyMap.find(xShape);
    assert(iter != mpXShapeToBodyMap.end());
    Box2DBodySharedPtr pBox2DBody = iter->second;
    pBox2DBody->setLinearVelocity(rVelocity);
}

void box2DWorld::setShapeAngleByAngularVelocity(
    const css::uno::Reference<css::drawing::XShape>& xShape, const double fAngle,
    const double fPassedTime)
{
    if (fPassedTime > 0) // this only makes sense if there was an advance in time
    {
        const auto iter = mpXShapeToBodyMap.find(xShape);
        assert(iter != mpXShapeToBodyMap.end());
        Box2DBodySharedPtr pBox2DBody = iter->second;
        pBox2DBody->setAngleByAngularVelocity(fAngle, fPassedTime);
    }
}

void box2DWorld::setShapeAngularVelocity(const css::uno::Reference<css::drawing::XShape>& xShape,
                                         const double fAngularVelocity)
{
    const auto iter = mpXShapeToBodyMap.find(xShape);
    assert(iter != mpXShapeToBodyMap.end());
    Box2DBodySharedPtr pBox2DBody = iter->second;
    pBox2DBody->setAngularVelocity(fAngularVelocity);
}

void box2DWorld::setShapeCollision(const css::uno::Reference<css::drawing::XShape>& xShape,
                                   bool bCanCollide)
{
    const auto iter = mpXShapeToBodyMap.find(xShape);
    assert(iter != mpXShapeToBodyMap.end());
    Box2DBodySharedPtr pBox2DBody = iter->second;
    pBox2DBody->setCollision(bCanCollide);
}

void box2DWorld::processUpdateQueue(const double fPassedTime)
{
    while (!maShapeParallelUpdateQueue.empty())
    {
        Box2DDynamicUpdateInformation& aQueueElement = maShapeParallelUpdateQueue.front();

        if (aQueueElement.mnDelayForSteps > 0)
        {
            // it was queued as a delayed action, skip it, don't pop
            aQueueElement.mnDelayForSteps--;
        }
        else
        {
            switch (aQueueElement.meUpdateType)
            {
                default:
                case BOX2D_UPDATE_POSITION_CHANGE:
                    setShapePositionByLinearVelocity(aQueueElement.mxShape,
                                                     aQueueElement.maPosition, fPassedTime);
                    break;
                case BOX2D_UPDATE_POSITION:
                    setShapePosition(aQueueElement.mxShape, aQueueElement.maPosition);
                    break;
                case BOX2D_UPDATE_ANGLE:
                    setShapeAngleByAngularVelocity(aQueueElement.mxShape, aQueueElement.mfAngle,
                                                   fPassedTime);
                    break;
                case BOX2D_UPDATE_SIZE:
                    break;
                case BOX2D_UPDATE_VISIBILITY:
                    setShapeCollision(aQueueElement.mxShape, aQueueElement.mbVisibility);
                    break;
                case BOX2D_UPDATE_LINEAR_VELOCITY:
                    setShapeLinearVelocity(aQueueElement.mxShape, aQueueElement.maVelocity);
                    break;
                case BOX2D_UPDATE_ANGULAR_VELOCITY:
                    setShapeAngularVelocity(aQueueElement.mxShape, aQueueElement.mfAngularVelocity);
            }
            maShapeParallelUpdateQueue.pop();
        }
    }
}

void box2DWorld::initiateAllShapesAsStaticBodies(
    const slideshow::internal::ShapeManagerSharedPtr& pShapeManager)
{
    mbShapesInitialized = true;
    auto aXShapeToShapeMap = pShapeManager->getXShapeToShapeMap();

    std::unordered_map<css::uno::Reference<css::drawing::XShape>, bool> aXShapeBelongsToAGroup;

    // iterate over the shapes in the current slide and flag them if they belong to a group
    // will flag the only ones that are belong to a group since std::unordered_map operator[]
    // defaults the value to false if the key doesn't have a corresponding value
    for (auto aIt = aXShapeToShapeMap.begin(); aIt != aXShapeToShapeMap.end(); aIt++)
    {
        slideshow::internal::ShapeSharedPtr pShape = aIt->second;
        if (pShape->isForeground())
        {
            SdrObject* pTemp = SdrObject::getSdrObjectFromXShape(pShape->getXShape());
            if (pTemp && pTemp->IsGroupObject())
            {
                // if it is a group object iterate over its children and flag them
                SdrObjList* aObjList = pTemp->GetSubList();

                for (const rtl::Reference<SdrObject>& pGroupMember : *aObjList)
                {
                    aXShapeBelongsToAGroup.insert(
                        std::make_pair(GetXShapeForSdrObject(pGroupMember.get()), true));
                }
            }
        }
    }

    // iterate over shapes in the current slide
    for (auto aIt = aXShapeToShapeMap.begin(); aIt != aXShapeToShapeMap.end(); aIt++)
    {
        slideshow::internal::ShapeSharedPtr pShape = aIt->second;
        // only create static bodies for the shapes that do not belong to a group
        // groups themselves will have one body that represents the whole shape
        // collection
        if (pShape->isForeground() && !aXShapeBelongsToAGroup[pShape->getXShape()])
        {
            Box2DBodySharedPtr pBox2DBody = createStaticBody(pShape);

            mpXShapeToBodyMap.insert(std::make_pair(pShape->getXShape(), pBox2DBody));
            if (!pShape->isVisible())
            {
                // if the shape isn't visible, queue an update for it
                queueShapeVisibilityUpdate(pShape->getXShape(), false);
            }
        }
    }
}

bool box2DWorld::hasWorldStepper() const { return mbHasWorldStepper; }

void box2DWorld::setHasWorldStepper(const bool bHasWorldStepper)
{
    mbHasWorldStepper = bHasWorldStepper;
}

void box2DWorld::queueDynamicPositionUpdate(const css::uno::Reference<css::drawing::XShape>& xShape,
                                            const basegfx::B2DPoint& rOutPos)
{
    Box2DDynamicUpdateInformation aQueueElement = { xShape, {}, BOX2D_UPDATE_POSITION_CHANGE };
    aQueueElement.maPosition = rOutPos;
    maShapeParallelUpdateQueue.push(aQueueElement);
}

void box2DWorld::queueLinearVelocityUpdate(const css::uno::Reference<css::drawing::XShape>& xShape,
                                           const basegfx::B2DVector& rVelocity,
                                           const int nDelayForSteps)
{
    Box2DDynamicUpdateInformation aQueueElement
        = { xShape, {}, BOX2D_UPDATE_LINEAR_VELOCITY, nDelayForSteps };
    aQueueElement.maVelocity = rVelocity;
    maShapeParallelUpdateQueue.push(aQueueElement);
}

void box2DWorld::queueDynamicRotationUpdate(const css::uno::Reference<css::drawing::XShape>& xShape,
                                            const double fAngle)
{
    Box2DDynamicUpdateInformation aQueueElement = { xShape, {}, BOX2D_UPDATE_ANGLE };
    aQueueElement.mfAngle = fAngle;
    maShapeParallelUpdateQueue.push(aQueueElement);
}

void box2DWorld::queueAngularVelocityUpdate(const css::uno::Reference<css::drawing::XShape>& xShape,
                                            const double fAngularVelocity, const int nDelayForSteps)
{
    Box2DDynamicUpdateInformation aQueueElement
        = { xShape, {}, BOX2D_UPDATE_ANGULAR_VELOCITY, nDelayForSteps };
    aQueueElement.mfAngularVelocity = fAngularVelocity;
    maShapeParallelUpdateQueue.push(aQueueElement);
}

void box2DWorld::queueShapeVisibilityUpdate(const css::uno::Reference<css::drawing::XShape>& xShape,
                                            const bool bVisibility)
{
    Box2DDynamicUpdateInformation aQueueElement = { xShape, {}, BOX2D_UPDATE_VISIBILITY };
    aQueueElement.mbVisibility = bVisibility;
    maShapeParallelUpdateQueue.push(aQueueElement);
}

void box2DWorld::queueShapePositionUpdate(const css::uno::Reference<css::drawing::XShape>& xShape,
                                          const basegfx::B2DPoint& rOutPos)
{
    Box2DDynamicUpdateInformation aQueueElement = { xShape, {}, BOX2D_UPDATE_POSITION };
    aQueueElement.maPosition = rOutPos;
    maShapeParallelUpdateQueue.push(aQueueElement);
}

void box2DWorld::queueShapePathAnimationUpdate(
    const css::uno::Reference<css::drawing::XShape>& xShape,
    const slideshow::internal::ShapeAttributeLayerSharedPtr& pAttrLayer, const bool bIsFirstUpdate)
{
    // Workaround for PathAnimations since they do not have their own AttributeType
    // - using PosX makes it register a DynamicPositionUpdate -
    queueShapeAnimationUpdate(xShape, pAttrLayer, slideshow::internal::AttributeType::PosX,
                              bIsFirstUpdate);
}

void box2DWorld::queueShapeAnimationUpdate(
    const css::uno::Reference<css::drawing::XShape>& xShape,
    const slideshow::internal::ShapeAttributeLayerSharedPtr& pAttrLayer,
    const slideshow::internal::AttributeType eAttrType, const bool bIsFirstUpdate)
{
    switch (eAttrType)
    {
        case slideshow::internal::AttributeType::Visibility:
            queueShapeVisibilityUpdate(xShape, pAttrLayer->getVisibility());
            return;
        case slideshow::internal::AttributeType::Rotate:
            queueDynamicRotationUpdate(xShape, pAttrLayer->getRotationAngle());
            return;
        case slideshow::internal::AttributeType::PosX:
        case slideshow::internal::AttributeType::PosY:
            if (bIsFirstUpdate) // if it is the first update shape should _teleport_ to the position
                queueShapePositionUpdate(xShape, { pAttrLayer->getPosX(), pAttrLayer->getPosY() });
            else
                queueDynamicPositionUpdate(xShape,
                                           { pAttrLayer->getPosX(), pAttrLayer->getPosY() });
            return;
        default:
            return;
    }
}

void box2DWorld::queueShapeAnimationEndUpdate(
    const css::uno::Reference<css::drawing::XShape>& xShape,
    const slideshow::internal::AttributeType eAttrType)
{
    switch (eAttrType)
    {
        // end updates that change the velocity are delayed for a step
        // since we do not want them to override the last position/angle
        case slideshow::internal::AttributeType::Rotate:
            queueAngularVelocityUpdate(xShape, 0.0, 1);
            return;
        case slideshow::internal::AttributeType::PosX:
        case slideshow::internal::AttributeType::PosY:
            queueLinearVelocityUpdate(xShape, { 0, 0 }, 1);
            return;
        default:
            return;
    }
}

void box2DWorld::alertPhysicsAnimationEnd(const slideshow::internal::ShapeSharedPtr& pShape)
{
    const auto iter = mpXShapeToBodyMap.find(pShape->getXShape());
    assert(iter != mpXShapeToBodyMap.end());
    Box2DBodySharedPtr pBox2DBody = iter->second;
    // since the animation ended make the body static
    makeBodyStatic(pBox2DBody);
    pBox2DBody->setRestitution(fDefaultStaticBodyBounciness);
    if (--mnPhysicsAnimationCounter == 0)
    {
        // if there are no more physics animation effects going on clean up
        maShapeParallelUpdateQueue = {};
        mbShapesInitialized = false;
        // clearing the map will make the box2d bodies get
        // destroyed if there's nothing else that owns them
        mpXShapeToBodyMap.clear();
    }
    else
    {
        // the physics animation that will take over the lock after this one
        // shouldn't step the world for an update cycle - since it was already
        // stepped.
        mbAlreadyStepped = true;
    }
}

void box2DWorld::alertPhysicsAnimationStart(
    const ::basegfx::B2DVector& rSlideSize,
    const slideshow::internal::ShapeManagerSharedPtr& pShapeManager)
{
#if BOX2D_CHECK_VERSION(3, 0)
    if (!mbWorldInitialized)
#else
    if (!mpBox2DWorld)
#endif
        createStaticFrameAroundSlide(rSlideSize);

    if (!mbShapesInitialized)
        initiateAllShapesAsStaticBodies(pShapeManager);

    mnPhysicsAnimationCounter++;
}

double box2DWorld::stepAmount(const double fPassedTime)
{
#if !BOX2D_CHECK_VERSION(3, 0)
    assert(mpBox2DWorld);
#endif

    // attention fTimeStep should not vary.
    const float fTimeStep = 1.0f / 100.0f;
#if BOX2D_CHECK_VERSION(3, 0)
    const int nSubStepCount = 4;
#else
    const int nVelocityIterations = 6;
    const int nPositionIterations = 2;
#endif

    unsigned int nStepAmount = static_cast<unsigned int>(std::round(fPassedTime / fTimeStep));
    // find the actual time that will be stepped through so
    // that the updates can be processed using that value
    double fTimeSteppedThrough = fTimeStep * nStepAmount;

    // do the updates required to simulate other animation effects going in parallel
    processUpdateQueue(fTimeSteppedThrough);

    if (!mbAlreadyStepped)
    {
        for (unsigned int nStepCounter = 0; nStepCounter < nStepAmount; nStepCounter++)
        {
#if BOX2D_CHECK_VERSION(3, 0)
            b2World_Step(maBox2DWorldId, fTimeStep, nSubStepCount);
#else
            mpBox2DWorld->Step(fTimeStep, nVelocityIterations, nPositionIterations);
#endif
        }
    }
    else
    {
        // just got the step lock from another physics animation
        // so skipping stepping the world for an update cycle
        mbAlreadyStepped = false;
    }

    return fTimeSteppedThrough;
}

bool box2DWorld::isInitialized() const
{
#if BOX2D_CHECK_VERSION(3, 0)
    if (mbWorldInitialized)
#else
    if (mpBox2DWorld)
#endif
        return true;
    else
        return false;
}

Box2DBodySharedPtr
box2DWorld::makeShapeDynamic(const css::uno::Reference<css::drawing::XShape>& xShape,
                             const basegfx::B2DVector& rStartVelocity, const double fDensity,
                             const double fBounciness)
{
#if !BOX2D_CHECK_VERSION(3, 0)
    assert(mpBox2DWorld);
#endif
    Box2DBodySharedPtr pBox2DBody = mpXShapeToBodyMap.find(xShape)->second;
    pBox2DBody->setDensityAndRestitution(fDensity, fBounciness);
    queueLinearVelocityUpdate(xShape, rStartVelocity, 1);
    return makeBodyDynamic(pBox2DBody);
}

Box2DBodySharedPtr makeBodyDynamic(const Box2DBodySharedPtr& pBox2DBody)
{
    if (pBox2DBody->getType() != BOX2D_DYNAMIC_BODY)
    {
        pBox2DBody->setType(BOX2D_DYNAMIC_BODY);
    }
    return pBox2DBody;
}

Box2DBodySharedPtr makeBodyStatic(const Box2DBodySharedPtr& pBox2DBody)
{
    if (pBox2DBody->getType() != BOX2D_STATIC_BODY)
    {
        pBox2DBody->setType(BOX2D_STATIC_BODY);
    }
    return pBox2DBody;
}

Box2DBodySharedPtr box2DWorld::createStaticBody(const slideshow::internal::ShapeSharedPtr& rShape)
{
    ::basegfx::B2DRectangle aShapeBounds = rShape->getBounds();

    b2BodyDef aBodyDef;
    aBodyDef.type = b2_staticBody;
    aBodyDef.position = convertB2DPointToBox2DVec2(aShapeBounds.getCenter(), mfScaleFactor);

    slideshow::internal::ShapeAttributeLayerSharedPtr pShapeAttributeLayer
        = static_cast<slideshow::internal::AttributableShape*>(rShape.get())
              ->getTopmostAttributeLayer();
    if (pShapeAttributeLayer && pShapeAttributeLayer->isRotationAngleValid())
    {
    // if the shape's rotation value was altered by another animation effect set it.
#if BOX2D_CHECK_VERSION(3, 0)
        aBodyDef.rotation
            = b2MakeRot(::basegfx::deg2rad(-pShapeAttributeLayer->getRotationAngle()));
    }

    auto pBody = std::shared_ptr<b2BodyId>(new b2BodyId(b2CreateBody(maBox2DWorldId, &aBodyDef)),
                                           [](b2BodyId* pB2BodyId) {
                                               if (pB2BodyId != nullptr)
                                               {
                                                   b2DestroyBody(*pB2BodyId);
                                                   delete pB2BodyId;
                                               }
                                           });

#else
        aBodyDef.angle = ::basegfx::deg2rad(-pShapeAttributeLayer->getRotationAngle());
    }

    // create a shared pointer with a destructor so that the body will be properly destroyed
    std::shared_ptr<b2Body> pBody(mpBox2DWorld->CreateBody(&aBodyDef), [](b2Body* pB2Body) {
        pB2Body->GetWorld()->DestroyBody(pB2Body);
    });
#endif

    SdrObject* pSdrObject = SdrObject::getSdrObjectFromXShape(rShape->getXShape());

    rtl::OUString aShapeType = rShape->getXShape()->getShapeType();

    basegfx::B2DPolyPolygon aPolyPolygon;
    // workaround:
    // TakeXorPoly() doesn't return beziers for CustomShapes and we want the beziers
    // so that we can decide the complexity of the polygons generated from them
    if (aShapeType == "com.sun.star.drawing.CustomShape")
    {
        aPolyPolygon = static_cast<SdrObjCustomShape*>(pSdrObject)->GetLineGeometry(true);
    }
    else
    {
        aPolyPolygon = pSdrObject->TakeXorPoly();
    }

    // make beziers into polygons, using a high degree angle as fAngleBound in
    // adaptiveSubdivideByAngle reduces complexity of the resulting polygon shapes
    aPolyPolygon = aPolyPolygon.areControlPointsUsed()
                       ? basegfx::utils::adaptiveSubdivideByAngle(aPolyPolygon, 20)
                       : aPolyPolygon;
    aPolyPolygon.removeDoublePoints();

    // make polygon coordinates relative to the center of the shape instead of top left of the slide
    // since box2d shapes are expressed this way
    aPolyPolygon
        = basegfx::utils::distort(aPolyPolygon, aPolyPolygon.getB2DRange(),
                                  { -aShapeBounds.getWidth() / 2, -aShapeBounds.getHeight() / 2 },
                                  { aShapeBounds.getWidth() / 2, -aShapeBounds.getHeight() / 2 },
                                  { -aShapeBounds.getWidth() / 2, aShapeBounds.getHeight() / 2 },
                                  { aShapeBounds.getWidth() / 2, aShapeBounds.getHeight() / 2 });

    if (pSdrObject->IsClosedObj() && !pSdrObject->IsEdgeObj() && pSdrObject->HasFillStyle())
    {
        basegfx::triangulator::B2DTriangleVector aTriangleVector;
        // iterate over the polygons of the shape and create representations for them
        for (const auto& rPolygon : std::as_const(aPolyPolygon))
        {
            // if the polygon is closed it will be represented by triangles
            if (rPolygon.isClosed())
            {
                basegfx::triangulator::B2DTriangleVector aTempTriangleVector(
                    basegfx::triangulator::triangulate(rPolygon));
                aTriangleVector.insert(aTriangleVector.end(), aTempTriangleVector.begin(),
                                       aTempTriangleVector.end());
            }
            else // otherwise it will be an edge representation (example: smile line of the smiley shape)
            {
                addEdgeShapeToBody(rPolygon, pBody.get(), mfScaleFactor);
            }
        }
        addTriangleVectorToBody(aTriangleVector, pBody.get(), mfScaleFactor);
    }
    else
    {
        addEdgeShapeToBody(aPolyPolygon, pBody.get(), mfScaleFactor);
    }

    return std::make_shared<box2DBody>(pBody, mfScaleFactor);
#if defined __GNUC__ && !defined __clang__ && __GNUC__ == 16
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#endif
}
#if defined __GNUC__ && !defined __clang__ && __GNUC__ == 16
#pragma GCC diagnostic pop
#endif

#if BOX2D_CHECK_VERSION(3, 0)
box2DBody::box2DBody(std::shared_ptr<b2BodyId> pBox2DBody, double fScaleFactor)
#else
box2DBody::box2DBody(std::shared_ptr<b2Body> pBox2DBody, double fScaleFactor)
#endif
    : mpBox2DBody(std::move(pBox2DBody))
    , mfScaleFactor(fScaleFactor)
{
}

::basegfx::B2DPoint box2DBody::getPosition() const
{
#if BOX2D_CHECK_VERSION(3, 0)
    b2Vec2 aPosition = b2Body_GetPosition(*mpBox2DBody);
#else
    b2Vec2 aPosition = mpBox2DBody->GetPosition();
#endif
    double fX = static_cast<double>(aPosition.x) / mfScaleFactor;
    double fY = static_cast<double>(aPosition.y) / -mfScaleFactor;
    return ::basegfx::B2DPoint(fX, fY);
}

void box2DBody::setPosition(const basegfx::B2DPoint& rPos)
{
#if BOX2D_CHECK_VERSION(3, 0)
    b2Body_SetTransform(*mpBox2DBody, convertB2DPointToBox2DVec2(rPos, mfScaleFactor),
                        b2Body_GetRotation(*mpBox2DBody));
#else
    mpBox2DBody->SetTransform(convertB2DPointToBox2DVec2(rPos, mfScaleFactor),
                              mpBox2DBody->GetAngle());
#endif
}

void box2DBody::setPositionByLinearVelocity(const basegfx::B2DPoint& rDesiredPos,
                                            const double fPassedTime)
{
// kinematic bodies are not affected by other bodies, but unlike static ones can still have velocity
#if BOX2D_CHECK_VERSION(3, 0)
    if (b2Body_GetType(*mpBox2DBody) != b2_kinematicBody)
        b2Body_SetType(*mpBox2DBody, b2_kinematicBody);
#else
    if (mpBox2DBody->GetType() != b2_kinematicBody)
        mpBox2DBody->SetType(b2_kinematicBody);
#endif

    ::basegfx::B2DPoint aCurrentPos = getPosition();
    // calculate the velocity needed to reach the rDesiredPos in the given time frame
    ::basegfx::B2DVector aVelocity = (rDesiredPos - aCurrentPos) / fPassedTime;

    setLinearVelocity(aVelocity);
}

void box2DBody::setAngleByAngularVelocity(const double fDesiredAngle, const double fPassedTime)
{
// kinematic bodies are not affected by other bodies, but unlike static ones can still have velocity
#if BOX2D_CHECK_VERSION(3, 0)
    if (b2Body_GetType(*mpBox2DBody) != b2_kinematicBody)
        b2Body_SetType(*mpBox2DBody, b2_kinematicBody);
#else
    if (mpBox2DBody->GetType() != b2_kinematicBody)
        mpBox2DBody->SetType(b2_kinematicBody);
#endif

    double fDeltaAngle = fDesiredAngle - getAngle();

    // temporary hack for repeating animation effects
    while (fDeltaAngle > 180
           || fDeltaAngle < -180) // if it is bigger than 180 opposite rotation is actually closer
        fDeltaAngle += fDeltaAngle > 0 ? -360 : +360;

    double fAngularVelocity = fDeltaAngle / fPassedTime;
    setAngularVelocity(fAngularVelocity);
}

void box2DBody::setLinearVelocity(const ::basegfx::B2DVector& rVelocity)
{
    b2Vec2 aVelocity = { static_cast<float>(rVelocity.getX() * mfScaleFactor),
                         static_cast<float>(rVelocity.getY() * -mfScaleFactor) };
#if BOX2D_CHECK_VERSION(3, 0)
    b2Body_SetLinearVelocity(*mpBox2DBody, aVelocity);
#else
    mpBox2DBody->SetLinearVelocity(aVelocity);
#endif
}

void box2DBody::setAngularVelocity(const double fAngularVelocity)
{
    float fBox2DAngularVelocity = static_cast<float>(basegfx::deg2rad(-fAngularVelocity));
#if BOX2D_CHECK_VERSION(3, 0)
    b2Body_SetAngularVelocity(*mpBox2DBody, fBox2DAngularVelocity);
#else
    mpBox2DBody->SetAngularVelocity(fBox2DAngularVelocity);
#endif
}

void box2DBody::setCollision(const bool bCanCollide)
{
#if BOX2D_CHECK_VERSION(3, 0)
    int nShapeCount = b2Body_GetShapeCount(*mpBox2DBody);
    std::vector<b2ShapeId> aShapeIds(nShapeCount);
    b2Body_GetShapes(*mpBox2DBody, aShapeIds.data(), nShapeCount);

    for (int i = 0; i < nShapeCount; ++i)
    {
        b2ShapeId aShapeId = aShapeIds[i];
        b2Filter aFilter = b2Shape_GetFilter(aShapeId);
        // 0xFFFF means collides with everything
        // 0x0000 means collides with nothing
        aFilter.maskBits = bCanCollide ? 0xFFFF : 0x0000;
        b2Shape_SetFilter(aShapeId, aFilter);
    }
#else
    // collision have to be set for each fixture of the body individually
    for (b2Fixture* pFixture = mpBox2DBody->GetFixtureList(); pFixture;
         pFixture = pFixture->GetNext())
    {
        b2Filter aFilter = pFixture->GetFilterData();
        // 0xFFFF means collides with everything
        // 0x0000 means collides with nothing
        aFilter.maskBits = bCanCollide ? 0xFFFF : 0x0000;
        pFixture->SetFilterData(aFilter);
    }
#endif
}

double box2DBody::getAngle() const
{
#if BOX2D_CHECK_VERSION(3, 0)
    double fAngle = static_cast<double>(b2Rot_GetAngle(b2Body_GetRotation(*mpBox2DBody)));
#else
    double fAngle = static_cast<double>(mpBox2DBody->GetAngle());
#endif
    return ::basegfx::rad2deg(-fAngle);
}

void box2DBody::setAngle(const double fAngle)
{
#if BOX2D_CHECK_VERSION(3, 0)
    b2Body_SetTransform(*mpBox2DBody, b2Body_GetPosition(*mpBox2DBody),
                        b2MakeRot(::basegfx::deg2rad(-fAngle)));
#else
    mpBox2DBody->SetTransform(mpBox2DBody->GetPosition(), ::basegfx::deg2rad(-fAngle));
#endif
}

void box2DBody::setDensityAndRestitution(const double fDensity, const double fRestitution)
{
#if BOX2D_CHECK_VERSION(3, 0)
    int nShapeCount = b2Body_GetShapeCount(*mpBox2DBody);
    std::vector<b2ShapeId> aShapeIds(nShapeCount);
    b2Body_GetShapes(*mpBox2DBody, aShapeIds.data(), nShapeCount);

    for (int i = 0; i < nShapeCount; ++i)
    {
        b2ShapeId aShapeId = aShapeIds[i];
#if BOX2D_CHECK_VERSION(3, 1)
        b2Shape_SetDensity(aShapeId, static_cast<float>(fDensity), false);
#else
        b2Shape_SetDensity(aShapeId, static_cast<float>(fDensity));
#endif
        b2Shape_SetRestitution(aShapeId, static_cast<float>(fRestitution));
    }
    // without resetting the massdata of the body, density change won't take effect
    b2Body_ApplyMassFromShapes(*mpBox2DBody);
#else
    // density and restitution have to be set for each fixture of the body individually
    for (b2Fixture* pFixture = mpBox2DBody->GetFixtureList(); pFixture;
         pFixture = pFixture->GetNext())
    {
        pFixture->SetDensity(static_cast<float>(fDensity));
        pFixture->SetRestitution(static_cast<float>(fRestitution));
    }
    // without resetting the massdata of the body, density change won't take effect
    mpBox2DBody->ResetMassData();
#endif
}

void box2DBody::setRestitution(const double fRestitution)
{
#if BOX2D_CHECK_VERSION(3, 0)
    int nShapeCount = b2Body_GetShapeCount(*mpBox2DBody);
    std::vector<b2ShapeId> aShapeIds(nShapeCount);
    b2Body_GetShapes(*mpBox2DBody, aShapeIds.data(), nShapeCount);

    for (int i = 0; i < nShapeCount; ++i)
    {
        b2ShapeId aShapeId = aShapeIds[i];
        b2Shape_SetRestitution(aShapeId, static_cast<float>(fRestitution));
    }
#else
    for (b2Fixture* pFixture = mpBox2DBody->GetFixtureList(); pFixture;
         pFixture = pFixture->GetNext())
    {
        pFixture->SetRestitution(static_cast<float>(fRestitution));
    }
#endif
}

void box2DBody::setType(box2DBodyType eType)
{
#if BOX2D_CHECK_VERSION(3, 0)
    b2Body_SetType(*mpBox2DBody, getBox2DInternalBodyType(eType));
#else
    mpBox2DBody->SetType(getBox2DInternalBodyType(eType));
#endif
}

box2DBodyType box2DBody::getType() const
{
#if BOX2D_CHECK_VERSION(3, 0)
    return getBox2DLOBodyType(b2Body_GetType(*mpBox2DBody));
#else
    return getBox2DLOBodyType(mpBox2DBody->GetType());
#endif
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
