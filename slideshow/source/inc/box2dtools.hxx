/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "shape.hxx"
#include "shapeattributelayer.hxx"
#include "attributemap.hxx"
#include <map>
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
class box2DBody;
class box2DWorld;
typedef std::shared_ptr<box2DWorld> Box2DWorldSharedPtr;
typedef std::shared_ptr<box2DBody> Box2DBodySharedPtr;

enum box2DBodyType
{
    BOX2D_STATIC_BODY = 0,
    BOX2D_KINEMATIC_BODY,
    BOX2D_DYNAMIC_BODY
};

enum box2DNonsimulatedShapeUpdateType
{
    BOX2D_UPDATE_POSITION_CHANGE,
    BOX2D_UPDATE_POSITION,
    BOX2D_UPDATE_ANGLE,
    BOX2D_UPDATE_SIZE,
    BOX2D_UPDATE_VISIBILITY,
    BOX2D_UPDATE_LINEAR_VELOCITY,
    BOX2D_UPDATE_ANGULAR_VELOCITY
};

/// Holds required information to perform an update to box2d
/// body of a shape that was altered by an animation effect
struct Box2DDynamicUpdateInformation
{
    /// reference to the shape that the update belongs to
    css::uno::Reference<css::drawing::XShape> mxShape;
    union {
        ::basegfx::B2DPoint maPosition;
        ::basegfx::B2DVector maVelocity;
        double mfAngle;
        double mfAngularVelocity;
        bool mbVisibility;
    };
    box2DNonsimulatedShapeUpdateType meUpdateType;
    /// amount of steps to delay the update for
    int mnDelayForSteps = 0;
};

/** Class that manages the Box2D World

    This class is used when there's a physics animation going on,
    it handles the stepping through the box2d world, updating the
    shapes in the box2d world if they were changed by ongoing animations.
 */
class box2DWorld
{
private:
    /// Pointer to the real Box2D World that this class manages
    std::unique_ptr<b2World> mpBox2DWorld;
    /// Scale factor for conversions between LO user space coordinates to Box2D World coordinates
    double mfScaleFactor;
    bool mbShapesInitialized;
    /// Holds whether or not there is a PhysicsAnimation that
    /// is stepping the Box2D World. Used to create a lock mechanism
    bool mbHasWorldStepper;
    /// Flag used to stop overstepping that occurs when a physics
    /// animation effect transfers step-lock to another one.
    bool mbAlreadyStepped;
    /// Number of Physics Animations going on
    int mnPhysicsAnimationCounter;
    std::unordered_map<css::uno::Reference<css::drawing::XShape>, Box2DBodySharedPtr>
        mpXShapeToBodyMap;

    /** Queue that holds any required information to keep LO animation effects
        and Box2DWorld in sync

        Is processed before every step of the box2d world by processUpdateQueue.
        Holds position, rotation, visibility etc. changes and associated values.
     */
    std::queue<Box2DDynamicUpdateInformation> maShapeParallelUpdateQueue;

    /// Creates a static frame in Box2D world that corresponds to the slide borders
    void createStaticFrameAroundSlide(const ::basegfx::B2DVector& rSlideSize);

    /** Sets shape's corresponding Box2D body to the specified position

        Body is teleported to the specified position, not moved

        @param xShape
        Shape reference

        @param rOutPos
        Position in LO user space coordinates
     */
    void setShapePosition(const css::uno::Reference<css::drawing::XShape> xShape,
                          const ::basegfx::B2DPoint& rOutPos);

    /** Moves shape's corresponding Box2D body to specified position

        Moves shape's corresponding Box2D body to specified position as if
        the body had velocity to reach that point in given time frame.

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

    /** Sets linear velocity of the shape's corresponding Box2D body

        Moves shape's corresponding Box2D body to specified position as if
        the body had velocity to reach that point in given time frame.

        @param xShape
        Shape reference

        @param rVelocity
        Velocity vector in LO user space coordinates.
     */
    void setShapeLinearVelocity(const css::uno::Reference<com::sun::star::drawing::XShape> xShape,
                                const basegfx::B2DVector& rVelocity);

    /** Sets rotation angle of the shape's corresponding Box2D body

        @param xShape
        Shape reference

        @param fAngle
        Angle of rotation in degrees.
     */
    void setShapeAngle(const css::uno::Reference<com::sun::star::drawing::XShape> xShape,
                       const double fAngle);

    /** Rotates shape's corresponding Box2D body to specified angle

        Rotates the Box2D body to specified angle as if the body
        had exact angular velocity to reach that point in given
        time frame

        @param xShape
        Shape reference

        @param fAngle
        Angle of rotation in degrees.

        @param fPassedTime
        Time frame which the Box2D body should rotate to the specified angle.
     */
    void setShapeAngleByAngularVelocity(
        const css::uno::Reference<com::sun::star::drawing::XShape> xShape, const double fAngle,
        const double fPassedTime);

    /** Sets angular velocity of the shape's corresponding Box2D body.

        @param xShape
        Shape reference

        @param fAngularVelocity
        Angular velocity in degrees per second.
     */
    void setShapeAngularVelocity(const css::uno::Reference<com::sun::star::drawing::XShape> xShape,
                                 const double fAngularVelocity);

    /** Sets whether a shape's corresponding Box2D body has collision in the Box2D World or not

        Used for animations that change the visibility of the shape.

        @param xShape
        Shape reference

        @param bCanCollide
        true if collisions should be enabled for the corresponding Box2D body of this shape
        and false if it should be disabled.
    */
    void setShapeCollision(const css::uno::Reference<com::sun::star::drawing::XShape> xShape,
                           const bool bCanCollide);

    /** Process the updates queued in the maShapeParallelUpdateQueue

        Called on each step of the box2DWorld.

        @param fPassedTime
        Time frame to process the updates accordingly (needed for proper simulations)
     */
    void processUpdateQueue(const double fPassedTime);

    /** Simulate and step through time in the Box2D World

        Used in stepAmount

        @attention fTimeStep should not vary.
     */
    void step(const float fTimeStep = 1.0f / 100.0f, const int nVelocityIterations = 6,
              const int nPositionIterations = 2);

    /// Queue a rotation update that is simulated as if shape's corresponding box2D body rotated to given angle when processed
    void
    queueDynamicRotationUpdate(const css::uno::Reference<com::sun::star::drawing::XShape>& xShape,
                               const double fAngle);

    /// Queue an angular velocity update that sets the shape's corresponding box2D body angular velocity to the given value when processed
    void
    queueAngularVelocityUpdate(const css::uno::Reference<com::sun::star::drawing::XShape>& xShape,
                               const double fAngularVelocity, const int nDelayForSteps = 0);

    /// Queue an collision update that sets the collision of shape's corresponding box2D body when processed
    void queueShapeVisibilityUpdate(const css::uno::Reference<css::drawing::XShape>& xShape,
                                    const bool bVisibility);

    void queueShapePositionUpdate(const css::uno::Reference<css::drawing::XShape>& xShape,
                                  const ::basegfx::B2DPoint& rOutPos);

public:
    box2DWorld(const ::basegfx::B2DVector& rSlideSize);
    ~box2DWorld();

    bool initiateWorld(const ::basegfx::B2DVector& rSlideSize);

    /** Simulate and step through a given amount of time in the Box2D World

        @param fPassedTime
        Amount of time to step through

        @return Amount of time actually stepped through, since it is possible
        to only step through a multiple of fTimeStep

        @attention fTimeStep should not vary.
    */
    double stepAmount(const double fPassedTime, const float fTimeStep = 1.0f / 100.0f,
                      const int nVelocityIterations = 6, const int nPositionIterations = 2);

    /// @return whether shapes in the slide are initialized as Box2D bodies or not
    bool shapesInitialized();
    /// @return whether the Box2D World is initialized or not
    bool isInitialized() const;

    /** Make the shape's corresponding box2D body a dynamic one.

        A dynamic body will be affected by other bodies and the gravity.

        @param xShape
        Shape reference

        @param rStartVelocity
        Velocity of the shape after making it dynamic

        @param fDensity
        Density of the body that is in kg/m^2

        @param fBounciness
        Bounciness of the body that is usually in between [0,1].
        Even though it could take values that are >1, it is way too chaotic.

        @return box2d body pointer
     */
    Box2DBodySharedPtr makeShapeDynamic(const css::uno::Reference<css::drawing::XShape>& xShape,
                                        const basegfx::B2DVector& rStartVelocity,
                                        const double fDensity, const double fBounciness);

    /** Make the Box2D body corresponding to the given shape a static one

        A static body will not be affected by other bodies and the gravity. But will
        affect other bodies that are dynamic (will still collide with them but won't
        move etc.)

        @param pShape
        Pointer to the shape to alter the corresponding Box2D body of

        @return box2d body pointer
     */
    Box2DBodySharedPtr makeShapeStatic(const slideshow::internal::ShapeSharedPtr& pShape);

    /** Create a static body that is represented by the shape's geometry

        @return pointer to the box2d body
     */
    Box2DBodySharedPtr createStaticBody(const slideshow::internal::ShapeSharedPtr& rShape,
                                        const float fDensity = 1.0f, const float fFriction = 0.3f);

    /// Initiate all the shapes in the current slide in the box2DWorld as static ones
    void
    initateAllShapesAsStaticBodies(const slideshow::internal::ShapeManagerSharedPtr& pShapeManager);

    /// @return whether the box2DWorld has a stepper or not
    bool hasWorldStepper() const;

    /// Set the flag for whether the box2DWorld has a stepper or not
    void setHasWorldStepper(const bool bHasWorldStepper);

    /// Queue a position update that is simulated as if shape's corresponding box2D body moved to given position when processed
    void queueDynamicPositionUpdate(const css::uno::Reference<css::drawing::XShape>& xShape,
                                    const ::basegfx::B2DPoint& rOutPos);

    /// Queue a update that sets the corresponding box2D body's linear velocity to the given value when processed
    void queueLinearVelocityUpdate(const css::uno::Reference<css::drawing::XShape>& xShape,
                                   const ::basegfx::B2DVector& rVelocity,
                                   const int nDelayForSteps = 0);

    /// Queue an appropriate update for the animation effect that is in parallel with a physics animation
    void
    queueShapeAnimationUpdate(const css::uno::Reference<css::drawing::XShape>& xShape,
                              const slideshow::internal::ShapeAttributeLayerSharedPtr& pAttrLayer,
                              const slideshow::internal::AttributeType eAttrType,
                              const bool bIsFirstUpdate);

    /// Queue an appropriate update for a path animation that is in parallel with a physics animation
    void queueShapePathAnimationUpdate(
        const css::uno::Reference<com::sun::star::drawing::XShape>& xShape,
        const slideshow::internal::ShapeAttributeLayerSharedPtr& pAttrLayer,
        const bool bIsFirstUpdate);

    /// Queue an appropriate update for the animation effect that just ended
    void queueShapeAnimationEndUpdate(const css::uno::Reference<css::drawing::XShape>& xShape,
                                      const slideshow::internal::AttributeType eAttrType);

    /** Alert that a physics animation effect has ended

        Makes the given shape static, if this was the last physics animation effect
        that was in parallel, box2d bodies that are owned by the mpXShapeToBodyMap
        are dumped and potentially destroyed.

        @attention the box2d body owned by the PhysicsAnimation won't be destroyed.
     */
    void alertPhysicsAnimationEnd(const slideshow::internal::ShapeSharedPtr& pShape);

    /** Alert that a physics animation effect has started

        Initiates the box2D world if it is not initiated yet, and likewise constructs
        box2d bodies for the shapes in the current slide if they are not constructed.
     */
    void
    alertPhysicsAnimationStart(const ::basegfx::B2DVector& rSlideSize,
                               const slideshow::internal::ShapeManagerSharedPtr& pShapeManager);
};

/// Class that manages a single box2D Body
class box2DBody
{
private:
    /// Pointer to the body that this class manages
    std::shared_ptr<b2Body> mpBox2DBody;
    /// Scale factor for conversions between LO user space coordinates to Box2D World coordinates
    double mfScaleFactor;

public:
    box2DBody(std::shared_ptr<b2Body> pBox2DBody, double fScaleFactor);

    /// @return current position in LO user space coordinates
    ::basegfx::B2DPoint getPosition() const;

    /** Set the position of box2d body

        @param rPos
        Position in LO user space coordinates
     */
    void setPosition(const ::basegfx::B2DPoint& rPos);

    /** Moves body to the specified position

        Moves body to the specified position by setting velocity of
        the body so that it reaches to rDesiredPos in given time fram

        @param rDesiredPos
        Position to arrive in the time frame

        @param fPassedTime
        Amount of time for the movement to take place
     */
    void setPositionByLinearVelocity(const ::basegfx::B2DPoint& rDesiredPos,
                                     const double fPassedTime);

    /** Sets linear velocity of the body

        @param rVelocity
        Velocity vector in LO user space coordinates
     */
    void setLinearVelocity(const ::basegfx::B2DVector& rVelocity);

    /** Rotate body to specified angle of rotation

        Rotates body to specified rotation as if the body had
        angular velocity to reach that state in given time frame

        @param fDesiredAngle
        Rotation angle in degrees to arrive in the time frame

        @param fPassedTime
        Amount of time for the movement to take place
     */
    void setAngleByAngularVelocity(const double fDesiredAngle, const double fPassedTime);

    /** Sets angular velocity of the body

        @param fAngularVelocity
        Angular velocity in degrees per second
     */
    void setAngularVelocity(const double fAngularVelocity);

    /// Sets whether the body have collisions or not
    void setCollision(const bool bCanCollide);

    /// @return current angle of rotation of the body
    double getAngle() const;

    /** Set angle of the box2d body

        @param fAngle
        Angle in degrees
     */
    void setAngle(const double fAngle);

    /** Set density and restitution of the box2d body

        @param fDensity
        Density in kg/m^2

        @param fRestitution
        Restitution (elasticity) coefficient, usually in the range [0,1]
     */
    void setDensityAndRestitution(const double fDensity, const double fRestitution);

    /** Set restitution of the box2d body

        @param fRestitution
        Restitution (elasticity) coefficient, usually in the range [0,1]
     */
    void setRestitution(const double fRestitution);

    /// Set type of the body
    void setType(box2DBodyType eType);

    /// @return type of the body
    box2DBodyType getType() const;
};

/** Make the Box2D body a dynamic one

    A dynamic body will be affected by other bodies and the gravity.

    @param pBox2DBody
    Pointer to the Box2D body
 */
Box2DBodySharedPtr makeBodyDynamic(const Box2DBodySharedPtr& pBox2DBody);

/** Make the Box2D body a static one

    A static body will not be affected by other bodies and the gravity.

    @param pBox2DBody
    Pointer to the Box2D body
 */
Box2DBodySharedPtr makeBodyStatic(const Box2DBodySharedPtr& pBox2DBody);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
