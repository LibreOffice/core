/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <box2dtools.hxx>
#include <Box2D/Box2D.h>

#include <shapemanager.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygontriangulator.hxx>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>

#include <svx/svdobj.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svdogrp.hxx>

#include <svx/unoapi.hxx>

#define BOX2D_SLIDE_SIZE_IN_METERS 100.00f
constexpr double fDefaultStaticBodyBounciness(0.1);

namespace box2d::utils
{
namespace
{
double calculateScaleFactor(const ::basegfx::B2DVector& rSlideSize)
{
    double fWidth = rSlideSize.getX();
    double fHeight = rSlideSize.getY();

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
                             b2Body* aBody, const float fDensity, const float fFriction,
                             const float fRestitution, const double fScaleFactor)
{
    for (const basegfx::triangulator::B2DTriangle& aTriangle : rTriangleVector)
    {
        b2FixtureDef aFixture;
        b2PolygonShape aPolygonShape;
        b2Vec2 aTriangleVertices[3]
            = { convertB2DPointToBox2DVec2(aTriangle.getA(), fScaleFactor),
                convertB2DPointToBox2DVec2(aTriangle.getB(), fScaleFactor),
                convertB2DPointToBox2DVec2(aTriangle.getC(), fScaleFactor) };

        bool bValidPointDistance = true;
        for (int nPointIndexA = 0; nPointIndexA < 3; nPointIndexA++)
        {
            for (int nPointIndexB = 0; nPointIndexB < 3; nPointIndexB++)
            {
                if (nPointIndexA == nPointIndexB)
                    continue;

                // check whether the triangle would be a degenerately small one
                if (b2DistanceSquared(aTriangleVertices[nPointIndexA],
                                      aTriangleVertices[nPointIndexB])
                    < 0.003f)
                {
                    bValidPointDistance = false;
                }
            }
        }
        if (bValidPointDistance)
        {
            aPolygonShape.Set(aTriangleVertices, 3);
            aFixture.shape = &aPolygonShape;
            aFixture.density = fDensity;
            aFixture.friction = fFriction;
            aFixture.restitution = fRestitution;
            aBody->CreateFixture(&aFixture);
        }
    }
}

// expects rPolygon to have coordinates relative to it's center
void addEdgeShapeToBody(const basegfx::B2DPolygon& rPolygon, b2Body* aBody, const float fDensity,
                        const float fFriction, const float fRestitution, const double fScaleFactor)
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
        b2FixtureDef aFixture;
        b2PolygonShape aPolygonShape;

        basegfx::B2DPoint aPointA;
        basegfx::B2DPoint aPointB;
        if (nIndex != 0)
        {
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

        b2Vec2 aEdgeUnitVec(convertB2DPointToBox2DVec2(aPointB, fScaleFactor)
                            - convertB2DPointToBox2DVec2(aPointA, fScaleFactor));
        aEdgeUnitVec.Normalize();

        b2Vec2 aEdgeNormal(-aEdgeUnitVec.y, aEdgeUnitVec.x);

        if (!bHasPreviousQuadrilateralEdge)
        {
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

        bool bValidPointDistance
            = b2DistanceSquared(aQuadrilateralVertices[0], aQuadrilateralVertices[2]) > 0.003f;

        if (bValidPointDistance)
        {
            aPolygonShape.Set(aQuadrilateralVertices, 4);
            aFixture.shape = &aPolygonShape;
            aFixture.density = fDensity;
            aFixture.friction = fFriction;
            aFixture.restitution = fRestitution;
            aBody->CreateFixture(&aFixture);

            // prepare the quadrilateral edge for next connection
            aQuadrilateralVertices[0] = aQuadrilateralVertices[2];
            aQuadrilateralVertices[1] = aQuadrilateralVertices[3];
        }
    }
}

void addEdgeShapeToBody(const basegfx::B2DPolyPolygon& rPolyPolygon, b2Body* aBody,
                        const float fDensity, const float fFriction, const float fRestitution,
                        const double fScaleFactor)
{
    for (const basegfx::B2DPolygon& rPolygon : rPolyPolygon)
    {
        addEdgeShapeToBody(rPolygon, aBody, fDensity, fFriction, fRestitution, fScaleFactor);
    }
}
}

box2DWorld::box2DWorld(const ::basegfx::B2DVector& rSlideSize)
    : mpBox2DWorld()
    , mfScaleFactor(calculateScaleFactor(rSlideSize))
    , mbShapesInitialized(false)
    , mbHasWorldStepper(false)
    , mpXShapeToBodyMap()
    , maShapeParallelUpdateQueue()
    , maShapeSequentialUpdate()
{
}

box2DWorld::~box2DWorld() = default;

bool box2DWorld::initiateWorld(const ::basegfx::B2DVector& rSlideSize)
{
    if (!mpBox2DWorld)
    {
        mpBox2DWorld = std::make_unique<b2World>(b2Vec2(0.0f, -30.0f));
        createStaticFrameAroundSlide(rSlideSize);
        return false;
    }
    else
    {
        return true;
    }
}

void box2DWorld::createStaticFrameAroundSlide(const ::basegfx::B2DVector& rSlideSize)
{
    assert(mpBox2DWorld);

    float fWidth = static_cast<float>(rSlideSize.getX() * mfScaleFactor);
    float fHeight = static_cast<float>(rSlideSize.getY() * mfScaleFactor);

    // static body for creating the frame around the slide
    b2BodyDef aBodyDef;
    aBodyDef.type = b2_staticBody;
    aBodyDef.position.Set(0, 0);

    // not going to be stored anywhere, Box2DWorld will handle this body
    b2Body* pStaticBody = mpBox2DWorld->CreateBody(&aBodyDef);

    // create an edge loop that represents slide frame
    b2Vec2 aEdgePoints[4];
    aEdgePoints[0].Set(0, 0);
    aEdgePoints[1].Set(0, -fHeight);
    aEdgePoints[2].Set(fWidth, -fHeight);
    aEdgePoints[3].Set(fWidth, 0);

    b2ChainShape aEdgesChainShape;
    aEdgesChainShape.CreateLoop(aEdgePoints, 4);

    b2FixtureDef aFixtureDef;
    aFixtureDef.shape = &aEdgesChainShape;
    pStaticBody->CreateFixture(&aFixtureDef);
}

void box2DWorld::setShapePosition(const css::uno::Reference<com::sun::star::drawing::XShape> xShape,
                                  const basegfx::B2DPoint& rOutPos)
{
    Box2DBodySharedPtr pBox2DBody = mpXShapeToBodyMap.find(xShape)->second;
    pBox2DBody->setPosition(rOutPos);
}

void box2DWorld::setShapePositionByLinearVelocity(
    const css::uno::Reference<com::sun::star::drawing::XShape> xShape,
    const basegfx::B2DPoint& rOutPos, const double fPassedTime)
{
    assert(mpBox2DWorld);
    if (fPassedTime > 0) // this only makes sense if there was an advance in time
    {
        Box2DBodySharedPtr pBox2DBody = mpXShapeToBodyMap.find(xShape)->second;
        pBox2DBody->setPositionByLinearVelocity(rOutPos, fPassedTime);
    }
}

void box2DWorld::setShapeLinearVelocity(
    const css::uno::Reference<com::sun::star::drawing::XShape> xShape,
    const basegfx::B2DVector& rVelocity)
{
    assert(mpBox2DWorld);
    Box2DBodySharedPtr pBox2DBody = mpXShapeToBodyMap.find(xShape)->second;
    pBox2DBody->setLinearVelocity(rVelocity);
}

void box2DWorld::setShapeAngle(const css::uno::Reference<com::sun::star::drawing::XShape> xShape,
                               const double fAngle)
{
    Box2DBodySharedPtr pBox2DBody = mpXShapeToBodyMap.find(xShape)->second;
    pBox2DBody->setAngle(fAngle);
}

void box2DWorld::setShapeAngleByAngularVelocity(
    const css::uno::Reference<com::sun::star::drawing::XShape> xShape, const double fAngle,
    const double fPassedTime)
{
    assert(mpBox2DWorld);
    if (fPassedTime > 0) // this only makes sense if there was an advance in time
    {
        Box2DBodySharedPtr pBox2DBody = mpXShapeToBodyMap.find(xShape)->second;
        pBox2DBody->setAngleByAngularVelocity(fAngle, fPassedTime);
    }
}

void box2DWorld::setShapeAngularVelocity(
    const css::uno::Reference<com::sun::star::drawing::XShape> xShape,
    const double fAngularVelocity)
{
    assert(mpBox2DWorld);
    Box2DBodySharedPtr pBox2DBody = mpXShapeToBodyMap.find(xShape)->second;
    pBox2DBody->setAngularVelocity(fAngularVelocity);
}

void box2DWorld::setShapeCollision(
    const css::uno::Reference<com::sun::star::drawing::XShape> xShape, bool bCanCollide)
{
    assert(mpBox2DWorld);
    Box2DBodySharedPtr pBox2DBody = mpXShapeToBodyMap.find(xShape)->second;
    pBox2DBody->setCollision(bCanCollide);
}

void box2DWorld::processUpdateQueue(const double fPassedTime)
{
    if (maShapeSequentialUpdate.empty())
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
                    case BOX2D_UPDATE_POSITION:
                        setShapePositionByLinearVelocity(aQueueElement.mxShape,
                                                         aQueueElement.maPosition, fPassedTime);
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
                        setShapeAngularVelocity(aQueueElement.mxShape,
                                                aQueueElement.mfAngularVelocity);
                }
                maShapeParallelUpdateQueue.pop();
            }
        }
    }
    else
    {
        // clear the Parallel Update Queue since the updates in it
        // are not relevant now - if there's any
        maShapeParallelUpdateQueue = {};

        for (auto& aIt : maShapeSequentialUpdate)
        {
            const css::uno::Reference<css::drawing::XShape>& xShape = aIt.first.first;
            const box2DNonsimulatedShapeUpdateType eUpdateType = aIt.first.second;
            const Box2DStaticUpdateInformation& rUpdateInformation = aIt.second;

            switch (eUpdateType)
            {
                default:
                case BOX2D_UPDATE_POSITION:
                    setShapePosition(xShape, rUpdateInformation.maPosition);
                    break;
                case BOX2D_UPDATE_ANGLE:
                    setShapeAngle(xShape, rUpdateInformation.mfAngle);
                    break;
                case BOX2D_UPDATE_VISIBILITY:
                    setShapeCollision(xShape, rUpdateInformation.mbVisibility);
                    break;
            }
        }

        // After applying all required updates empty map
        maShapeSequentialUpdate.clear();
    }
}

void box2DWorld::initateAllShapesAsStaticBodies(
    const slideshow::internal::ShapeManagerSharedPtr& pShapeManager)
{
    assert(mpBox2DWorld);

    mbShapesInitialized = true;
    auto aXShapeToShapeMap = pShapeManager->getXShapeToShapeMap();

    std::unordered_map<css::uno::Reference<css::drawing::XShape>, bool> aXShapeBelongsToAGroup;

    // iterate over the shapes in the current slide and flag them if they belong to a group
    for (auto aIt = aXShapeToShapeMap.begin(); aIt != aXShapeToShapeMap.end(); aIt++)
    {
        slideshow::internal::ShapeSharedPtr pShape = aIt->second;
        if (pShape->isForeground())
        {
            SdrObject* pTemp = SdrObject::getSdrObjectFromXShape(pShape->getXShape());
            if (pTemp && pTemp->IsGroupObject())
            {
                SdrObjList* aObjList = pTemp->GetSubList();
                const size_t nObjCount(aObjList->GetObjCount());

                for (size_t nObjIndex = 0; nObjIndex < nObjCount; ++nObjIndex)
                {
                    SdrObject* pGroupMember(aObjList->GetObj(nObjIndex));
                    aXShapeBelongsToAGroup.insert(
                        std::make_pair(GetXShapeForSdrObject(pGroupMember), true));
                }
            }
        }
    }

    // iterate over shapes in the current slide
    for (auto aIt = aXShapeToShapeMap.begin(); aIt != aXShapeToShapeMap.end(); aIt++)
    {
        slideshow::internal::ShapeSharedPtr pShape = aIt->second;
        if (pShape->isForeground() && !aXShapeBelongsToAGroup[pShape->getXShape()])
        {
            Box2DBodySharedPtr pBox2DBody = createStaticBody(pShape);

            mpXShapeToBodyMap.insert(std::make_pair(pShape->getXShape(), pBox2DBody));
            if (!pShape->isVisible())
            {
                // if the shape isn't visible, mark it
                queueShapeVisibilityUpdate(pShape->getXShape(), false);
            }
        }
    }
}

bool box2DWorld::hasWorldStepper() { return mbHasWorldStepper; }

void box2DWorld::setHasWorldStepper(const bool bHasWorldStepper)
{
    mbHasWorldStepper = bHasWorldStepper;
}

void box2DWorld::queueDynamicPositionUpdate(
    const css::uno::Reference<com::sun::star::drawing::XShape>& xShape,
    const basegfx::B2DPoint& rOutPos)
{
    Box2DDynamicUpdateInformation aQueueElement = { xShape, {}, BOX2D_UPDATE_POSITION };
    aQueueElement.maPosition = rOutPos;
    maShapeParallelUpdateQueue.push(aQueueElement);
}

void box2DWorld::queueLinearVelocityUpdate(
    const css::uno::Reference<com::sun::star::drawing::XShape>& xShape,
    const basegfx::B2DVector& rVelocity, const int nDelayForSteps)
{
    Box2DDynamicUpdateInformation aQueueElement
        = { xShape, {}, BOX2D_UPDATE_LINEAR_VELOCITY, nDelayForSteps };
    aQueueElement.maVelocity = rVelocity;
    maShapeParallelUpdateQueue.push(aQueueElement);
}

void box2DWorld::queueDynamicRotationUpdate(
    const css::uno::Reference<com::sun::star::drawing::XShape>& xShape, const double fAngle)
{
    Box2DDynamicUpdateInformation aQueueElement = { xShape, {}, BOX2D_UPDATE_ANGLE };
    aQueueElement.mfAngle = fAngle;
    maShapeParallelUpdateQueue.push(aQueueElement);
}

void box2DWorld::queueAngularVelocityUpdate(
    const css::uno::Reference<com::sun::star::drawing::XShape>& xShape,
    const double fAngularVelocity, const int nDelayForSteps)
{
    Box2DDynamicUpdateInformation aQueueElement
        = { xShape, {}, BOX2D_UPDATE_ANGULAR_VELOCITY, nDelayForSteps };
    aQueueElement.mfAngularVelocity = fAngularVelocity;
    maShapeParallelUpdateQueue.push(aQueueElement);
}

void box2DWorld::queueShapeVisibilityUpdate(
    const css::uno::Reference<com::sun::star::drawing::XShape>& xShape, const bool bVisibility)
{
    Box2DDynamicUpdateInformation aQueueElement = { xShape, {}, BOX2D_UPDATE_VISIBILITY };
    aQueueElement.mbVisibility = bVisibility;
    maShapeParallelUpdateQueue.push(aQueueElement);
}

void box2DWorld::queueShapePathAnimationUpdate(
    const css::uno::Reference<com::sun::star::drawing::XShape>& xShape,
    const slideshow::internal::ShapeAttributeLayerSharedPtr& pAttrLayer)
{
    // Workaround for PathAnimations since they do not have their own AttributeType
    // - using PosX makes it register a DynamicPositionUpdate -
    queueShapeAnimationUpdate(xShape, pAttrLayer, slideshow::internal::AttributeType::PosX);
}

void box2DWorld::queueShapeAnimationUpdate(
    const css::uno::Reference<com::sun::star::drawing::XShape>& xShape,
    const slideshow::internal::ShapeAttributeLayerSharedPtr& pAttrLayer,
    const slideshow::internal::AttributeType eAttrType)
{
    if (mbHasWorldStepper) // if there's a physics animation going on
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
                queueDynamicPositionUpdate(xShape,
                                           { pAttrLayer->getPosX(), pAttrLayer->getPosY() });
                return;
            default:
                return;
        }
    }
    else
    {
        Box2DStaticUpdateInformation aStaticUpdateInformation;
        switch (eAttrType)
        {
            case slideshow::internal::AttributeType::Visibility:
                aStaticUpdateInformation.mbVisibility = pAttrLayer->getVisibility();
                maShapeSequentialUpdate[std::make_pair(xShape, BOX2D_UPDATE_VISIBILITY)]
                    = aStaticUpdateInformation;
                return;
            case slideshow::internal::AttributeType::Rotate:
                aStaticUpdateInformation.mfAngle = pAttrLayer->getRotationAngle();
                maShapeSequentialUpdate[std::make_pair(xShape, BOX2D_UPDATE_ANGLE)]
                    = aStaticUpdateInformation;
                return;
            case slideshow::internal::AttributeType::PosX:
            case slideshow::internal::AttributeType::PosY:
                aStaticUpdateInformation.maPosition
                    = basegfx::B2DPoint(pAttrLayer->getPosX(), pAttrLayer->getPosY());
                maShapeSequentialUpdate[std::make_pair(xShape, BOX2D_UPDATE_POSITION)]
                    = aStaticUpdateInformation;
                return;
            default:
                return;
        }
    }
}

void box2DWorld::queueShapeAnimationEndUpdate(
    const css::uno::Reference<com::sun::star::drawing::XShape>& xShape,
    const slideshow::internal::AttributeType eAttrType)
{
    switch (eAttrType)
    {
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

void box2DWorld::alertAnimationEndForShape(const slideshow::internal::ShapeSharedPtr& pShape)
{
    Box2DBodySharedPtr pBox2DBody = mpXShapeToBodyMap.find(pShape->getXShape())->second;
    makeBodyStatic(pBox2DBody);
    pBox2DBody->setRestitution(fDefaultStaticBodyBounciness);
}

void box2DWorld::step(const float fTimeStep, const int nVelocityIterations,
                      const int nPositionIterations)
{
    assert(mpBox2DWorld);
    mpBox2DWorld->Step(fTimeStep, nVelocityIterations, nPositionIterations);
}

double box2DWorld::stepAmount(const double fPassedTime, const float fTimeStep,
                              const int nVelocityIterations, const int nPositionIterations)
{
    assert(mpBox2DWorld);

    unsigned int nStepAmount = static_cast<unsigned int>(std::round(fPassedTime / fTimeStep));
    double fTimeSteppedThrough = fTimeStep * nStepAmount;

    processUpdateQueue(fTimeSteppedThrough);

    for (unsigned int nStepCounter = 0; nStepCounter < nStepAmount; nStepCounter++)
    {
        step(fTimeStep, nVelocityIterations, nPositionIterations);
    }

    return fTimeSteppedThrough;
}

bool box2DWorld::shapesInitialized() { return mbShapesInitialized; }

bool box2DWorld::isInitialized()
{
    if (mpBox2DWorld)
        return true;
    else
        return false;
}

Box2DBodySharedPtr
box2DWorld::makeShapeDynamic(const css::uno::Reference<css::drawing::XShape>& xShape,
                             const basegfx::B2DVector& rStartVelocity, const double fDensity,
                             const double fBounciness)
{
    assert(mpBox2DWorld);
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

Box2DBodySharedPtr box2DWorld::makeShapeStatic(const slideshow::internal::ShapeSharedPtr& pShape)
{
    assert(mpBox2DWorld);
    Box2DBodySharedPtr pBox2DBody = mpXShapeToBodyMap.find(pShape->getXShape())->second;
    return makeBodyStatic(pBox2DBody);
}

Box2DBodySharedPtr makeBodyStatic(const Box2DBodySharedPtr& pBox2DBody)
{
    if (pBox2DBody->getType() != BOX2D_STATIC_BODY)
    {
        pBox2DBody->setType(BOX2D_STATIC_BODY);
    }
    return pBox2DBody;
}

Box2DBodySharedPtr box2DWorld::createStaticBody(const slideshow::internal::ShapeSharedPtr& rShape,
                                                const float fDensity, const float fFriction)
{
    assert(mpBox2DWorld);

    ::basegfx::B2DRectangle aShapeBounds = rShape->getBounds();

    b2BodyDef aBodyDef;
    aBodyDef.type = b2_staticBody;
    aBodyDef.position = convertB2DPointToBox2DVec2(aShapeBounds.getCenter(), mfScaleFactor);

    std::shared_ptr<b2Body> pBody(mpBox2DWorld->CreateBody(&aBodyDef), [](b2Body* pB2Body) {
        pB2Body->GetWorld()->DestroyBody(pB2Body);
    });

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
    aPolyPolygon
        = basegfx::utils::distort(aPolyPolygon, aPolyPolygon.getB2DRange(),
                                  { -aShapeBounds.getWidth() / 2, -aShapeBounds.getHeight() / 2 },
                                  { aShapeBounds.getWidth() / 2, -aShapeBounds.getHeight() / 2 },
                                  { -aShapeBounds.getWidth() / 2, aShapeBounds.getHeight() / 2 },
                                  { aShapeBounds.getWidth() / 2, aShapeBounds.getHeight() / 2 });

    if (pSdrObject->IsClosedObj() && !pSdrObject->IsEdgeObj() && pSdrObject->HasFillStyle())
    {
        basegfx::triangulator::B2DTriangleVector aTriangleVector;
        for (auto& rPolygon : aPolyPolygon)
        {
            if (rPolygon.isClosed())
            {
                basegfx::triangulator::B2DTriangleVector aTempTriangleVector(
                    basegfx::triangulator::triangulate(rPolygon));
                aTriangleVector.insert(aTriangleVector.end(), aTempTriangleVector.begin(),
                                       aTempTriangleVector.end());
            }
            else
            {
                addEdgeShapeToBody(rPolygon, pBody.get(), fDensity, fFriction,
                                   static_cast<float>(fDefaultStaticBodyBounciness), mfScaleFactor);
            }
        }
        addTriangleVectorToBody(aTriangleVector, pBody.get(), fDensity, fFriction,
                                static_cast<float>(fDefaultStaticBodyBounciness), mfScaleFactor);
    }
    else
    {
        addEdgeShapeToBody(aPolyPolygon, pBody.get(), fDensity, fFriction,
                           static_cast<float>(fDefaultStaticBodyBounciness), mfScaleFactor);
    }

    return std::make_shared<box2DBody>(pBody, mfScaleFactor);
}

box2DBody::box2DBody(std::shared_ptr<b2Body> pBox2DBody, double fScaleFactor)
    : mpBox2DBody(pBox2DBody)
    , mfScaleFactor(fScaleFactor)
{
}

::basegfx::B2DPoint box2DBody::getPosition()
{
    b2Vec2 aPosition = mpBox2DBody->GetPosition();
    double fX = static_cast<double>(aPosition.x) / mfScaleFactor;
    double fY = static_cast<double>(aPosition.y) / -mfScaleFactor;
    return ::basegfx::B2DPoint(fX, fY);
}

void box2DBody::setPosition(const basegfx::B2DPoint& rPos)
{
    mpBox2DBody->SetTransform(convertB2DPointToBox2DVec2(rPos, mfScaleFactor),
                              mpBox2DBody->GetAngle());
}

void box2DBody::setPositionByLinearVelocity(const basegfx::B2DPoint& rDesiredPos,
                                            const double fPassedTime)
{
    if (mpBox2DBody->GetType() != b2_kinematicBody)
        mpBox2DBody->SetType(b2_kinematicBody);

    ::basegfx::B2DPoint aCurrentPos = getPosition();
    ::basegfx::B2DVector aVelocity = (rDesiredPos - aCurrentPos) / fPassedTime;

    setLinearVelocity(aVelocity);
}

void box2DBody::setAngleByAngularVelocity(const double fDesiredAngle, const double fPassedTime)
{
    if (mpBox2DBody->GetType() != b2_kinematicBody)
        mpBox2DBody->SetType(b2_kinematicBody);

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
    mpBox2DBody->SetLinearVelocity(aVelocity);
}

void box2DBody::setAngularVelocity(const double fAngularVelocity)
{
    float fBox2DAngularVelocity = static_cast<float>(basegfx::deg2rad(-fAngularVelocity));
    mpBox2DBody->SetAngularVelocity(fBox2DAngularVelocity);
}

void box2DBody::setCollision(const bool bCanCollide)
{
    for (b2Fixture* pFixture = mpBox2DBody->GetFixtureList(); pFixture;
         pFixture = pFixture->GetNext())
    {
        b2Filter aFilter = pFixture->GetFilterData();
        aFilter.maskBits = bCanCollide ? 0xFFFF : 0x0000;
        pFixture->SetFilterData(aFilter);
    }
}

double box2DBody::getAngle()
{
    double fAngle = static_cast<double>(mpBox2DBody->GetAngle());
    return ::basegfx::rad2deg(-fAngle);
}

void box2DBody::setAngle(const double fAngle)
{
    mpBox2DBody->SetTransform(mpBox2DBody->GetPosition(), ::basegfx::deg2rad(-fAngle));
}

void box2DBody::setDensityAndRestitution(const double fDensity, const double fRestitution)
{
    for (b2Fixture* pFixture = mpBox2DBody->GetFixtureList(); pFixture;
         pFixture = pFixture->GetNext())
    {
        pFixture->SetDensity(static_cast<float>(fDensity));
        pFixture->SetRestitution(static_cast<float>(fRestitution));
    }
    mpBox2DBody->ResetMassData();
}

void box2DBody::setRestitution(const double fRestitution)
{
    for (b2Fixture* pFixture = mpBox2DBody->GetFixtureList(); pFixture;
         pFixture = pFixture->GetNext())
    {
        pFixture->SetRestitution(static_cast<float>(fRestitution));
    }
}

void box2DBody::setType(box2DBodyType eType)
{
    mpBox2DBody->SetType(getBox2DInternalBodyType(eType));
}

box2DBodyType box2DBody::getType() { return getBox2DLOBodyType(mpBox2DBody->GetType()); }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
