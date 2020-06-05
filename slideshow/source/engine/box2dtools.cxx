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

#define BOX2D_SLIDE_SIZE_IN_METERS 100.00f

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
}

box2DWorld::box2DWorld(const ::basegfx::B2DVector& rSlideSize)
    : mpBox2DWorld()
    , mfScaleFactor(calculateScaleFactor(rSlideSize))
    , mbShapesInitialized(false)
    , mbHasWorldStepper(false)
    , mpXShapeToBodyMap()
    , maShapeUpdateQueue()
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
    while (!maShapeUpdateQueue.empty())
    {
        Box2DShapeUpdateInformation& aQueueElement = maShapeUpdateQueue.front();

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
                    setShapeAngularVelocity(aQueueElement.mxShape, aQueueElement.mfAngularVelocity);
            }
            maShapeUpdateQueue.pop();
        }
    }
}

void box2DWorld::initateAllShapesAsStaticBodies(
    const slideshow::internal::ShapeManagerSharedPtr pShapeManager)
{
    assert(mpBox2DWorld);

    mbShapesInitialized = true;
    auto aXShapeToShapeMap = pShapeManager->getXShapeToShapeMap();

    // iterate over shapes in the current slide
    for (auto aIt = aXShapeToShapeMap.begin(); aIt != aXShapeToShapeMap.end(); aIt++)
    {
        slideshow::internal::ShapeSharedPtr pShape = aIt->second;
        if (pShape->isForeground())
        {
            Box2DBodySharedPtr pBox2DBody = createStaticBodyFromBoundingBox(pShape);
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

void box2DWorld::queuePositionUpdate(css::uno::Reference<com::sun::star::drawing::XShape> xShape,
                                     const basegfx::B2DPoint& rOutPos)
{
    Box2DShapeUpdateInformation aQueueElement = { xShape, {}, BOX2D_UPDATE_POSITION };
    aQueueElement.maPosition = rOutPos;
    maShapeUpdateQueue.push(aQueueElement);
}

void box2DWorld::queueLinearVelocityUpdate(
    css::uno::Reference<com::sun::star::drawing::XShape> xShape,
    const basegfx::B2DVector& rVelocity)
{
    Box2DShapeUpdateInformation aQueueElement = { xShape, {}, BOX2D_UPDATE_LINEAR_VELOCITY, 1 };
    aQueueElement.maVelocity = rVelocity;
    maShapeUpdateQueue.push(aQueueElement);
}

void box2DWorld::queueRotationUpdate(css::uno::Reference<com::sun::star::drawing::XShape> xShape,
                                     const double fAngle)
{
    Box2DShapeUpdateInformation aQueueElement = { xShape, {}, BOX2D_UPDATE_ANGLE };
    aQueueElement.mfAngle = fAngle;
    maShapeUpdateQueue.push(aQueueElement);
}

void box2DWorld::queueAngularVelocityUpdate(
    css::uno::Reference<com::sun::star::drawing::XShape> xShape, const double fAngularVelocity)
{
    Box2DShapeUpdateInformation aQueueElement = { xShape, {}, BOX2D_UPDATE_ANGULAR_VELOCITY, 1 };
    aQueueElement.mfAngularVelocity = fAngularVelocity;
    maShapeUpdateQueue.push(aQueueElement);
}

void box2DWorld::queueShapeVisibilityUpdate(
    css::uno::Reference<com::sun::star::drawing::XShape> xShape, const bool bVisibility)
{
    Box2DShapeUpdateInformation aQueueElement = { xShape, {}, BOX2D_UPDATE_VISIBILITY };
    aQueueElement.mbVisibility = bVisibility;
    maShapeUpdateQueue.push(aQueueElement);
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

Box2DBodySharedPtr box2DWorld::makeShapeDynamic(const slideshow::internal::ShapeSharedPtr pShape)
{
    assert(mpBox2DWorld);
    Box2DBodySharedPtr pBox2DBody = mpXShapeToBodyMap.find(pShape->getXShape())->second;
    return makeBodyDynamic(pBox2DBody);
}

Box2DBodySharedPtr box2DWorld::makeBodyDynamic(const Box2DBodySharedPtr pBox2DBody)
{
    assert(mpBox2DWorld);
    if (pBox2DBody->getType() != BOX2D_DYNAMIC_BODY)
    {
        pBox2DBody->setType(BOX2D_DYNAMIC_BODY);
    }
    return pBox2DBody;
}

Box2DBodySharedPtr box2DWorld::makeShapeStatic(const slideshow::internal::ShapeSharedPtr pShape)
{
    assert(mpBox2DWorld);
    Box2DBodySharedPtr pBox2DBody = mpXShapeToBodyMap.find(pShape->getXShape())->second;
    return makeBodyStatic(pBox2DBody);
}

Box2DBodySharedPtr box2DWorld::makeBodyStatic(const Box2DBodySharedPtr pBox2DBody)
{
    assert(mpBox2DWorld);
    if (pBox2DBody->getType() != BOX2D_STATIC_BODY)
    {
        pBox2DBody->setType(BOX2D_STATIC_BODY);
    }
    return pBox2DBody;
}

Box2DBodySharedPtr
box2DWorld::createStaticBodyFromBoundingBox(const slideshow::internal::ShapeSharedPtr& rShape,
                                            const float fDensity, const float fFriction)
{
    assert(mpBox2DWorld);
    ::basegfx::B2DRectangle aShapeBounds = rShape->getBounds();
    double fShapeWidth = aShapeBounds.getWidth() * mfScaleFactor;
    double fShapeHeight = aShapeBounds.getHeight() * mfScaleFactor;

    b2BodyDef aBodyDef;
    aBodyDef.type = b2_staticBody;
    aBodyDef.position = convertB2DPointToBox2DVec2(aShapeBounds.getCenter(), mfScaleFactor);

    std::shared_ptr<b2Body> pBody(mpBox2DWorld->CreateBody(&aBodyDef), [](b2Body* pB2Body) {
        pB2Body->GetWorld()->DestroyBody(pB2Body);
    });

    b2PolygonShape aDynamicBox;
    aDynamicBox.SetAsBox(static_cast<float>(fShapeWidth / 2), static_cast<float>(fShapeHeight / 2));

    b2FixtureDef aFixtureDef;
    aFixtureDef.shape = &aDynamicBox;
    aFixtureDef.density = fDensity;
    aFixtureDef.friction = fFriction;
    aFixtureDef.restitution = 0.1f;

    pBody->CreateFixture(&aFixtureDef);
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

void box2DBody::setType(box2DBodyType eType)
{
    mpBox2DBody->SetType(getBox2DInternalBodyType(eType));
}

box2DBodyType box2DBody::getType() { return getBox2DLOBodyType(mpBox2DBody->GetType()); }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
