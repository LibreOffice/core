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

#include <box2dtools.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <sal/log.hxx>

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

box2DWorld::box2DWorld(const ::basegfx::B2DVector& rSlideSize, const float32& fGravityX,
                       const float32& fGravityY)
    : maBox2DWorld(b2Vec2(fGravityX, fGravityY))
    , mfScaleFactor(calculateScaleFactor(rSlideSize))
{
    createStaticFrameAroundSlide(rSlideSize);
}

b2Body* box2DWorld::createStaticFrameAroundSlide(const ::basegfx::B2DVector& rSlideSize)
{
    float32 fWidth = static_cast<float32>(rSlideSize.getX() * mfScaleFactor);
    // hardcoding fHeight momenteraly since rSlideSize appears to be a square
    // i will need something other than rSlideSize to frame the slide properly
    float32 fHeight = BOX2D_SLIDE_SIZE_IN_METERS * 9 / 16;

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

void box2DWorld::step(const float32& fTimeStep, const int32& nVelocityIterations,
                      const int32& nPositionIterations)
{
    maBox2DWorld.Step(fTimeStep, nVelocityIterations, nPositionIterations);
}

b2Body* box2DWorld::createBodyFromShapeBoundingBox(
    const slideshow::internal::AnimatableShapeSharedPtr& rShape,
    const slideshow::internal::ShapeAttributeLayerSharedPtr& rAttrLayer, const float32& fDensity,
    const float32& fFriction)
{
    ::basegfx::B2DRectangle aShapeBounds = rShape->getBounds();
    double fShapeWidth = aShapeBounds.getWidth() * mfScaleFactor;
    double fShapeHeight = aShapeBounds.getHeight() * mfScaleFactor;

    double fRotationAngle = ::basegfx::deg2rad(-rAttrLayer->getRotationAngle());

    ::basegfx::B2DPoint aShapePosition = aShapeBounds.getCenter();
    float32 fBodyPosX = aShapePosition.getX() * mfScaleFactor;
    float32 fBodyPosY = aShapePosition.getY() * -mfScaleFactor;

    b2BodyDef aBodyDef;
    aBodyDef.type = b2_dynamicBody;
    aBodyDef.position.Set(fBodyPosX, fBodyPosY);
    aBodyDef.angle = static_cast<float32>(fRotationAngle);

    b2Body* pBody = maBox2DWorld.CreateBody(&aBodyDef);

    b2PolygonShape aDynamicBox;
    aDynamicBox.SetAsBox(static_cast<float32>(fShapeWidth / 2),
                         static_cast<float32>(fShapeHeight / 2));

    b2FixtureDef aFixtureDef;
    aFixtureDef.shape = &aDynamicBox;
    aFixtureDef.density = fDensity;
    aFixtureDef.friction = fFriction;

    pBody->CreateFixture(&aFixtureDef);
    return pBody;
}

box2DBody::box2DBody(b2Body* pBox2DBody)
    : mpBox2DBody(pBox2DBody){};

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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
