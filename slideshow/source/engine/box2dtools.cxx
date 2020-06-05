/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <Box2D/Box2D.h>
#include <box2dtools.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <animatableshape.hxx>
#include <shapeattributelayer.hxx>
#include <sal/log.hxx>
#include <shapemanager.hxx>

namespace box2d::utils
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

box2DWorld::box2DWorld(const ::basegfx::B2DVector& rSlideSize, const float fGravityX,
                       const float fGravityY)
    : maBox2DWorld(b2Vec2(fGravityX, fGravityY))
    , mfScaleFactor(calculateScaleFactor(rSlideSize))
    , mbShapesInitialized(false)
    , mbHasWorldStepper(false)
    , mnDynamicShapeCount(0)
    , maXShapeToBodyMapPtr()
{
    createStaticFrameAroundSlide(rSlideSize);
}

b2Body* box2DWorld::createStaticFrameAroundSlide(const ::basegfx::B2DVector& rSlideSize)
{
    float fWidth = static_cast<float>(rSlideSize.getX() * mfScaleFactor);
    float fHeight = static_cast<float>(rSlideSize.getY() * mfScaleFactor);

    // static body for creating the frame made out of four walls
    b2BodyDef aBodyDef;
    aBodyDef.type = b2_staticBody;
    aBodyDef.position.Set(0, 0);
    b2Body* pStaticBody = maBox2DWorld.CreateBody(&aBodyDef);

    b2PolygonShape aPolygonShape;

    b2FixtureDef aFixtureDef;
    aFixtureDef.shape = &aPolygonShape;

    //add four walls to the static body
    aPolygonShape.SetAsBox(fWidth / 2.0f, 1.0f / 10.0f, // ground wall
                           b2Vec2(fWidth / 2, -fHeight - 1.0f / 10.0f), 0);
    pStaticBody->CreateFixture(&aFixtureDef);

    aPolygonShape.SetAsBox(fWidth / 2.0f, 1.0f / 10.0f, // ceiling wall
                           b2Vec2(fWidth / 2, 1.0f / 10.0f), 0);
    pStaticBody->CreateFixture(&aFixtureDef);

    aPolygonShape.SetAsBox(1.0f / 10.0f, fHeight / 2, // left wall
                           b2Vec2(-1.0f / 10.0f, -fHeight / 2), 0);
    pStaticBody->CreateFixture(&aFixtureDef);

    aPolygonShape.SetAsBox(1.0f / 10.0f, fHeight / 2.0f, // right wall
                           b2Vec2(fWidth + 1.0f / 10.0f, -fHeight / 2), 0);
    pStaticBody->CreateFixture(&aFixtureDef);

    return pStaticBody;
}

void box2DWorld::initateAllShapesAsStaticBodies(
    const slideshow::internal::ShapeManagerSharedPtr pShapeManager)
{
    mbShapesInitialized = true;
    auto aXShapeToShapeMap = pShapeManager->getXShapeToShapeMap();

    // iterate over shapes in the current slide
    for (auto aIt = aXShapeToShapeMap.begin(); aIt != aXShapeToShapeMap.end(); aIt++)
    {
        slideshow::internal::ShapeSharedPtr pShape = aIt->second;
        if (pShape->isVisible() && pShape->isForeground())
        {
            Box2DBodySharedPtr pBox2DBody = createStaticBodyFromBoundingBox(pShape);
            maXShapeToBodyMapPtr.insert(std::make_pair(pShape->getXShape(), pBox2DBody));
        }
    }
}

bool box2DWorld::hasWorldStepper() { return mbHasWorldStepper; }

void box2DWorld::setHasWorldStepper(bool bHasWorldStepper) { mbHasWorldStepper = bHasWorldStepper; }

void box2DWorld::step(const float fTimeStep, const int nVelocityIterations,
                      const int nPositionIterations)
{
    maBox2DWorld.Step(fTimeStep, nVelocityIterations, nPositionIterations);
}

bool box2DWorld::getShapesInitialized() { return mbShapesInitialized; }

Box2DBodySharedPtr box2DWorld::makeShapeDynamic(const slideshow::internal::ShapeSharedPtr pShape)
{
    Box2DBodySharedPtr pBox2DBody = maXShapeToBodyMapPtr.find(pShape->getXShape())->second;
    if (pBox2DBody->getType() != b2_dynamicBody)
    {
        pBox2DBody->setType(b2_dynamicBody);
        mnDynamicShapeCount++;
    }
    return pBox2DBody;
}

Box2DBodySharedPtr box2DWorld::makeBodyDynamic(const Box2DBodySharedPtr pBox2DBody)
{
    if (pBox2DBody->getType() != b2_dynamicBody)
    {
        pBox2DBody->setType(b2_dynamicBody);
        mnDynamicShapeCount++;
    }
    return pBox2DBody;
}

Box2DBodySharedPtr box2DWorld::makeShapeStatic(const slideshow::internal::ShapeSharedPtr pShape)
{
    Box2DBodySharedPtr pBox2DBody = maXShapeToBodyMapPtr.find(pShape->getXShape())->second;
    if (pBox2DBody->getType() != b2_staticBody)
    {
        pBox2DBody->setType(b2_staticBody);
        mnDynamicShapeCount--;
    }
    return pBox2DBody;
}

Box2DBodySharedPtr box2DWorld::makeBodyStatic(const Box2DBodySharedPtr pBox2DBody)
{
    if (pBox2DBody->getType() != b2_staticBody)
    {
        pBox2DBody->setType(b2_staticBody);
        mnDynamicShapeCount--;
    }
    return pBox2DBody;
}

Box2DBodySharedPtr box2DWorld::createDynamicBodyFromBoundingBox(
    const slideshow::internal::ShapeSharedPtr& rShape,
    const slideshow::internal::ShapeAttributeLayerSharedPtr& rAttrLayer, const float fDensity,
    const float fFriction)
{
    ::basegfx::B2DRectangle aShapeBounds = rShape->getBounds();
    double fShapeWidth = aShapeBounds.getWidth() * mfScaleFactor;
    double fShapeHeight = aShapeBounds.getHeight() * mfScaleFactor;

    double fRotationAngle = ::basegfx::deg2rad(-rAttrLayer->getRotationAngle());

    ::basegfx::B2DPoint aShapePosition = aShapeBounds.getCenter();
    float fBodyPosX = aShapePosition.getX() * mfScaleFactor;
    float fBodyPosY = aShapePosition.getY() * -mfScaleFactor;

    b2BodyDef aBodyDef;
    aBodyDef.type = b2_dynamicBody;
    aBodyDef.position.Set(fBodyPosX, fBodyPosY);
    aBodyDef.angle = static_cast<float>(fRotationAngle);

    b2Body* pBody = maBox2DWorld.CreateBody(&aBodyDef);

    b2PolygonShape aDynamicBox;
    aDynamicBox.SetAsBox(static_cast<float>(fShapeWidth / 2), static_cast<float>(fShapeHeight / 2));

    b2FixtureDef aFixtureDef;
    aFixtureDef.shape = &aDynamicBox;
    aFixtureDef.density = fDensity;
    aFixtureDef.friction = fFriction;

    pBody->CreateFixture(&aFixtureDef);
    return std::make_shared<box2DBody>(pBody);
}

Box2DBodySharedPtr
box2DWorld::createStaticBodyFromBoundingBox(const slideshow::internal::ShapeSharedPtr& rShape,
                                            const float fDensity, const float fFriction)
{
    ::basegfx::B2DRectangle aShapeBounds = rShape->getBounds();
    double fShapeWidth = aShapeBounds.getWidth() * mfScaleFactor;
    double fShapeHeight = aShapeBounds.getHeight() * mfScaleFactor;

    ::basegfx::B2DPoint aShapePosition = aShapeBounds.getCenter();
    float fBodyPosX = aShapePosition.getX() * mfScaleFactor;
    float fBodyPosY = aShapePosition.getY() * -mfScaleFactor;

    b2BodyDef aBodyDef;
    aBodyDef.type = b2_staticBody;
    aBodyDef.position.Set(fBodyPosX, fBodyPosY);

    b2Body* pBody = maBox2DWorld.CreateBody(&aBodyDef);

    b2PolygonShape aDynamicBox;
    aDynamicBox.SetAsBox(static_cast<float>(fShapeWidth / 2), static_cast<float>(fShapeHeight / 2));

    b2FixtureDef aFixtureDef;
    aFixtureDef.shape = &aDynamicBox;
    aFixtureDef.density = fDensity;
    aFixtureDef.friction = fFriction;

    pBody->CreateFixture(&aFixtureDef);
    return std::make_shared<box2DBody>(pBody);
}

box2DBody::box2DBody(b2Body* pBox2DBody /*, Box2DWorldSharedPtr pBox2DWorldSharedPtr*/)
    : mpBox2DBody(pBox2DBody)
    /* , mpBox2DWorldSharedPtr(pBox2DWorldSharedPtr)*/ {};

box2DBody::~box2DBody() { mpBox2DBody->GetWorld()->DestroyBody(mpBox2DBody); }

::basegfx::B2DPoint box2DBody::getPosition(const ::basegfx::B2DVector& rSlideSize)
{
    double fScaleFactor = calculateScaleFactor(rSlideSize);
    b2Vec2 aPosition = mpBox2DBody->GetPosition();
    double fX = static_cast<double>(aPosition.x) / fScaleFactor;
    double fY = static_cast<double>(aPosition.y) / -fScaleFactor;
    return ::basegfx::B2DPoint(fX, fY);
}

double box2DBody::getAngle()
{
    double fAngle = static_cast<double>(mpBox2DBody->GetAngle());
    return ::basegfx::rad2deg(-fAngle);
}

void box2DBody::setType(b2BodyType aType) { mpBox2DBody->SetType(aType); }

b2BodyType box2DBody::getType() { return mpBox2DBody->GetType(); }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
