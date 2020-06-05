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

#pragma once

#include <Box2D/Box2D.h>
#include "animatableshape.hxx"
#include "shapeattributelayer.hxx"

#define BOX2D_SLIDE_SIZE_IN_METERS 100.00f

namespace box2d::utils
{
double calculateScaleFactor(const ::basegfx::B2DVector& rSlideSize);

class box2DWorld
{
private:
    b2World maBox2DWorld;
    double mfScaleFactor;

    b2Body* createStaticFrameAroundSlide(const ::basegfx::B2DVector& rSlideSize);

public:
    box2DWorld(const ::basegfx::B2DVector& rSlideSize, const float fGravityX = 0.0f,
               const float fGravityY = -10.0f);

    box2DWorld(box2DWorld&) = delete;

    ~box2DWorld() = default;

    void step(const float fTimeStep = 1.0f / 60.0f, const int nVelocityIterations = 6,
              const int nPositionIterations = 2);

    b2Body* createDynamicBodyFromBoundingBox(
        const slideshow::internal::ShapeSharedPtr& rShape,
        const slideshow::internal::ShapeAttributeLayerSharedPtr& rAttrLayer,
        const float fDensity = 1.0f, const float fFriction = 0.3f);

    b2Body* createStaticBodyFromBoundingBox(const slideshow::internal::ShapeSharedPtr& rShape,
                                            const float fDensity = 1.0f,
                                            const float fFriction = 0.3f);
};

class box2DBody
{
private:
    b2Body* mpBox2DBody;

public:
    box2DBody(b2Body* pBox2DBody);

    ::basegfx::B2DPoint getPosition(const ::basegfx::B2DVector& rSlideSize);
    double getAngle();
};

typedef std::shared_ptr<box2DWorld> Box2DWorldSharedPtr;
typedef std::shared_ptr<box2DBody> Box2DBodySharedPtr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
