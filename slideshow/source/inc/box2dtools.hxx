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

#include "shape.hxx"
#include "shapeattributelayer.hxx"
#include <unordered_map>
#include <queue>

class b2Body;
class b2World;

namespace slideshow::internal
{
class ShapeManager;
typedef std::shared_ptr<ShapeManager> ShapeManagerSharedPtr;
}

namespace box2d::utils
{
enum box2DBodyType
{
    BOX2D_STATIC_BODY = 0,
    BOX2D_KINEMATIC_BODY,
    BOX2D_DYNAMIC_BODY
};

class box2DBody;
class box2DWorld;
typedef std::shared_ptr<box2DWorld> Box2DWorldSharedPtr;
typedef std::shared_ptr<box2DBody> Box2DBodySharedPtr;

struct Box2DShapeUpdateInformation;

/** Class that manages the Box2D World

    This class is used when there's a simulated animation going on,
    it handles the stepping through the simulated world, updating the
    shapes in the simulated world if they were changed by ongoing animations.
 */
class box2DWorld
{
private:
    /// Pointer to the real Box2D World that this class manages for simulations
    std::unique_ptr<b2World> mpBox2DWorld;
    /// Scale factor for conversions between LO user space coordinates to Box2D World coordinates
    double mfScaleFactor;
    bool mbShapesInitialized;
    bool mbHasWorldStepper;
    std::unordered_map<css::uno::Reference<css::drawing::XShape>, Box2DBodySharedPtr>
        mpXShapeToBodyMap;
    /// Holds any information needed to keep LO animations and Box2D world in sync
    std::queue<Box2DShapeUpdateInformation> maShapeUpdateQueue;

    /// Creates a static frame in Box2D world that corresponds to the slide borders
    void createStaticFrameAroundSlide(const ::basegfx::B2DVector& rSlideSize);

    /** Sets shape's corresponding Box2D body to specified position

        Sets shape's corresponding Box2D body to specified position as if
        the body had velocity to reach that point in given time frame

        @param xShape
        Shape reference

        @param rOutPos
        Position in LO user space coordinates

        @param fPassedTime
        Time frame which the Box2D body should move to the specified position.
     */
    void setShapePositionByLinearVelocity(const css::uno::Reference<css::drawing::XShape> xShape,
                                          const ::basegfx::B2DPoint& rOutPos,
                                          const double fPassedTime);
    /// Sets linear velocity of the shape's corresponding body in the Box2D world
    void setShapeLinearVelocity(const css::uno::Reference<com::sun::star::drawing::XShape> xShape,
                                const basegfx::B2DVector& rVelocity);

    /** Sets shape's corresponding Box2D body to specified angle

        Sets shape's corresponding Box2D body to specified angle as if
        the body had angular velocity to reach that point in given time frame

        @param xShape
        Shape reference

        @param fAngle
        Position in LO user space coordinates

        @param fPassedTime
        Time frame which the Box2D body should move to the specified position.
     */
    void setShapeAngleByAngularVelocity(
        const css::uno::Reference<com::sun::star::drawing::XShape> xShape, const double fAngle,
        const double fPassedTime);

    /// Sets angular velocity of the shape's corresponding body in the Box2D world
    void setShapeAngularVelocity(const css::uno::Reference<com::sun::star::drawing::XShape> xShape,
                                 const double fAngularVelocity);

    /** Set whether a shape can have collision in the Box2D World

        Used for animations that change the visibility of the shape.

        @param xShape
        Shape reference

        @param bCanCollide
        true if collisions should be enabled for the corresponding Box2D body of this shape
        and false if it should be disabled.
    */
    void setShapeCollision(const css::uno::Reference<com::sun::star::drawing::XShape> xShape,
                           const bool bCanCollide);
    /** Process the updates queued in the maShapeUpdateQueue

        Called on each step of the box2DWorld.

        @param fPassedTime
        Time frame to process the updates accordingly (needed for proper simulations)
     */
    void processUpdateQueue(const double fPassedTime);

    /// Simulate and step through time in the Box2D World
    void step(const float fTimeStep = 1.0f / 100.0f, const int nVelocityIterations = 6,
              const int nPositionIterations = 2);

public:
    box2DWorld(const ::basegfx::B2DVector& rSlideSize);
    ~box2DWorld();

    bool initiateWorld(const ::basegfx::B2DVector& rSlideSize);

    /** Simulate and step through a given amount of time in the Box2D World

        @param fPassedTime
        Amount of time to step through
    */
    double stepAmount(double fPassedTime, const float fTimeStep = 1.0f / 100.0f,
                      const int nVelocityIterations = 6, const int nPositionIterations = 2);

    /// @return whether shapes in the slide are initialized as Box2D bodies or not
    bool shapesInitialized();
    /// @return whether the Box2D shape is initialized or not
    bool isInitialized();

    /** Make the Box2D body corresponding to the given shape a dynamic one

        A dynamic body will be affected by other bodies and the gravity.

        @param pShape
        Pointer to the shape to alter the corresponding Box2D body of
     */
    Box2DBodySharedPtr makeShapeDynamic(const slideshow::internal::ShapeSharedPtr pShape);

    /** Make the Box2D body a dynamic one

        A dynamic body will be affected by other bodies and the gravity.

        @param pBox2DBody
        Pointer to the Box2D body
     */
    Box2DBodySharedPtr makeBodyDynamic(const Box2DBodySharedPtr pBox2DBody);

    /** Make the Box2D body corresponding to the given shape a static one

        A static body will not be affected by other bodies and the gravity.

        @param pShape
        Pointer to the shape to alter the corresponding Box2D body of
     */
    Box2DBodySharedPtr makeShapeStatic(const slideshow::internal::ShapeSharedPtr pShape);

    /** Make the Box2D body a dynamic one

        A static body will not be affected by other bodies and the gravity.

        @param pBox2DBody
        Pointer to the Box2D body
     */
    Box2DBodySharedPtr makeBodyStatic(const Box2DBodySharedPtr pBox2DBody);

    /// Create a dynamic body from the given shape's bounding box
    Box2DBodySharedPtr createDynamicBodyFromBoundingBox(
        const slideshow::internal::ShapeSharedPtr& rShape,
        const slideshow::internal::ShapeAttributeLayerSharedPtr& rAttrLayer,
        const float fDensity = 1.0f, const float fFriction = 0.3f);

    /// Create a static body from the given shape's bounding box
    Box2DBodySharedPtr
    createStaticBodyFromBoundingBox(const slideshow::internal::ShapeSharedPtr& rShape,
                                    const float fDensity = 1.0f, const float fFriction = 0.3f);

    /// Initiate all the shapes in the current slide in the box2DWorld as static ones
    void
    initateAllShapesAsStaticBodies(const slideshow::internal::ShapeManagerSharedPtr pShapeManager);

    /// @return whether the box2DWorld has a stepper or not
    bool hasWorldStepper();

    /// Set the flag for whether the box2DWorld has a stepper or not
    void setHasWorldStepper(bool bHasWorldStepper);

    /// Query a position update the next step of the box2DWorld for the corresponding body
    void queryPositionUpdate(css::uno::Reference<css::drawing::XShape> xShape,
                             const ::basegfx::B2DPoint& rOutPos);

    /// Query a linear velocity update on the next step of the box2DWorld for the corresponding body
    void queryLinearVelocityUpdate(css::uno::Reference<css::drawing::XShape> xShape,
                                   const ::basegfx::B2DVector& rVelocity);

    /// Query a rotation update on the next step of the box2DWorld for the corresponding body
    void queryRotationUpdate(css::uno::Reference<com::sun::star::drawing::XShape> xShape,
                             const double fAngle);

    /// Query an angular velocity update on the next step of the box2DWorld for the corresponding body
    void queryAngularVelocityUpdate(css::uno::Reference<com::sun::star::drawing::XShape> xShape,
                                    const double fAngularVelocity);

    /// Query an update that changes collision of the corresponding body on the next step of the box2DWorld,
    /// used for animations that change visibility
    void queryShapeVisibilityUpdate(css::uno::Reference<css::drawing::XShape> xShape,
                                    const bool bVisibility);
};

/// Class that manages a single box2D Body
class box2DBody
{
private:
    /// Pointer to the body that this class manages
    b2Body* mpBox2DBody;
    /// Scale factor for conversions between LO user space coordinates to Box2D World coordinates
    double mfScaleFactor;

public:
    box2DBody(b2Body* pBox2DBody, double fScaleFactor);

    ~box2DBody();

    /// @return current position in LO user space coordinates
    ::basegfx::B2DPoint getPosition();

    /** Sets body to specified position

        Sets body to specified position as if the body had
        velocity to reach that point in given time frame

        @param rDesiredPos
        Position to arrive in the time frame

        @param fPassedTime
        Amount of time for the movement to take place
     */
    void setPositionByLinearVelocity(const ::basegfx::B2DPoint& rDesiredPos,
                                     const double fPassedTime);

    /// Sets linear velocity of the body
    void setLinearVelocity(const ::basegfx::B2DVector& rVelocity);

    /** Sets body to specified angle of rotation

        Sets body to specified rotation as if the body had
        angular velocity to reach that state in given time frame

        @param fDesiredAngle
        Rotation angle to arrive in the time frame

        @param fPassedTime
        Amount of time for the movement to take place
     */
    void setAngleByAngularVelocity(const double fDesiredAngle, const double fPassedTime);

    /// Sets angular velocity of the body
    void setAngularVelocity(const double fAngularVelocity);

    /// Sets whether the body have collisions or not
    void setCollision(const bool bCanCollide);

    /// @return current angle of rotation of the body
    double getAngle();

    /// Set type of the body
    void setType(box2DBodyType eType);

    /// @return type of the body
    box2DBodyType getType();
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
