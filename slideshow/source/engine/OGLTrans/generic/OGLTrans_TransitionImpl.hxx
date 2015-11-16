/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef INCLUDED_OGLTRANS_TRANSITIONIMPL_HXX_
#define INCLUDED_OGLTRANS_TRANSITIONIMPL_HXX_

#include <config_lgpl.h>
#include <glm/glm.hpp>

#include <boost/noncopyable.hpp>

#include <GL/glew.h>

#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/vector/b3dvector.hxx>

#include <memory>
#include <vector>

class Primitive;
class Operation;
class SceneObject;
class TransitionData;

struct TransitionSettings
{
    TransitionSettings() :
        mbUseMipMapLeaving( true ),
        mbUseMipMapEntering( true ),
        mnRequiredGLVersion( 1.0 ),
        mbReflectSlides( false )
    {
    }

    /** Whether to use mipmaping for slides textures
     */
    bool mbUseMipMapLeaving;
    bool mbUseMipMapEntering;

    /** which GL version does the transition require
     */
    float mnRequiredGLVersion;

    /** Whether to reflect slides, the reflection happens on flat surface beneath the slides.
     ** Now it only works with slides which keep their rectangular shape together.
     */
    bool mbReflectSlides;
};

typedef std::vector<Primitive> Primitives_t;
typedef std::vector<std::shared_ptr<SceneObject> > SceneObjects_t;
typedef std::vector<std::shared_ptr<Operation> > Operations_t;

class TransitionScene
{
public:
    TransitionScene()
    {
    }

    TransitionScene(
            const Primitives_t& rLeavingSlidePrimitives,
            const Primitives_t& rEnteringSlidePrimitives,
            const Operations_t& rOverallOperations = Operations_t(),
            const SceneObjects_t& rSceneObjects = SceneObjects_t()
    )
        : maLeavingSlidePrimitives(rLeavingSlidePrimitives)
        , maEnteringSlidePrimitives(rEnteringSlidePrimitives)
        , maOverallOperations(rOverallOperations)
        , maSceneObjects(rSceneObjects)
    {
    }

    TransitionScene(TransitionScene const& rOther);
    TransitionScene& operator=(const TransitionScene& rOther);

    void swap(TransitionScene& rOther);

    const Primitives_t& getLeavingSlide() const
    {
        return maLeavingSlidePrimitives;
    }

    const Primitives_t& getEnteringSlide() const
    {
        return maEnteringSlidePrimitives;
    }

    const Operations_t& getOperations() const
    {
        return maOverallOperations;
    }

    const SceneObjects_t& getSceneObjects() const
    {
        return maSceneObjects;
    }

private:
    /** All the primitives that use the leaving slide texture
    */
    Primitives_t maLeavingSlidePrimitives;

    /** All the primitives that use the leaving slide texture
    */
    Primitives_t maEnteringSlidePrimitives;

    /** All the operations that should be applied to both leaving and entering slide primitives. These operations will be called in the order they were pushed back in. In OpenGL this effectively uses the operations in the opposite order they were pushed back.
    */
    Operations_t maOverallOperations;

    /** All the surrounding scene objects
    */
    SceneObjects_t maSceneObjects;
};

/** OpenGL 3D Transition class. It implicitly is constructed from XOGLTransition

    It holds Primitives and Operations on those primitives.
*/
class OGLTransitionImpl : private boost::noncopyable
{
public:
    virtual ~OGLTransitionImpl();

    /** Prepare transition.
      */
    void prepare( sal_Int32 glLeavingSlideTex, sal_Int32 glEnteringSlideTex );
    /** Display a step of the transition.
      */
    void display( double nTime, sal_Int32 glLeavingSlideTex, sal_Int32 glEnteringSlideTex, double SlideWidth, double SlideHeight, double DispWidth, double DispHeight );
    /** Clean up after transition.
      */
    void finish();

    TransitionSettings const& getSettings() const
    {
        return maSettings;
    }

protected:
    OGLTransitionImpl(const TransitionScene& rScene, const TransitionSettings& rSettings)
        : maScene(rScene)
        , maSettings(rSettings)
    {}

    OGLTransitionImpl() {}

    TransitionScene const& getScene() const
    {
        return maScene;
    }

    void setScene(TransitionScene const& rScene);
    // void setSettings(TransitionSettings const& rSettings);

    void displaySlide( double nTime, sal_Int32 glSlideTex, const Primitives_t& primitives, double SlideWidthScale, double SlideHeightScale );
    void displayScene( double nTime, double SlideWidth, double SlideHeight, double DispWidth, double DispHeight);
    void applyOverallOperations( double nTime, double SlideWidthScale, double SlideHeightScale );

private:
    /** This function is called in display method to prepare the slides, scene, etc.
      *
      * Default implementation does nothing.
      */
    virtual void prepare( double nTime, double SlideWidth, double SlideHeight, double DispWidth, double DispHeight );

    /** This function is called in display method to prepare the slides, scene, etc.
      *
      * Default implementation does nothing.
      */
    virtual void finish( double nTime, double SlideWidth, double SlideHeight, double DispWidth, double DispHeight );

    /** This function is called after glx context is ready to let the transition prepare GL related things, like GLSL program.
      *
      * Default implementation does nothing.
      */
    virtual void prepareTransition( sal_Int32 glLeavingSlideTex, sal_Int32 glEnteringSlideTex );

    /** This function is called when the transition needs to clear after itself, like delete own textures etc.
      *
      * Default implementation does nothing.
      */
    virtual void finishTransition();

    /** This function is called in display method to display the slides.
      *
      * Default implementation applies overall operations and then
      * displays both slides.
      */
    virtual void displaySlides_( double nTime, sal_Int32 glLeavingSlideTex, sal_Int32 glEnteringSlideTex, double SlideWidthScale, double SlideHeightScale );

private:
    TransitionScene maScene;
    const TransitionSettings maSettings;
};


// "Constructors" of available transitions
std::shared_ptr<OGLTransitionImpl> makeOutsideCubeFaceToLeft();
std::shared_ptr<OGLTransitionImpl> makeInsideCubeFaceToLeft();
std::shared_ptr<OGLTransitionImpl> makeNByMTileFlip( sal_uInt16 n, sal_uInt16 m );
std::shared_ptr<OGLTransitionImpl> makeRevolvingCircles( sal_uInt16 nCircles , sal_uInt16 nPointsOnCircles );
std::shared_ptr<OGLTransitionImpl> makeHelix( sal_uInt16 nRows );
std::shared_ptr<OGLTransitionImpl> makeFallLeaving();
std::shared_ptr<OGLTransitionImpl> makeTurnAround();
std::shared_ptr<OGLTransitionImpl> makeTurnDown();
std::shared_ptr<OGLTransitionImpl> makeIris();
std::shared_ptr<OGLTransitionImpl> makeRochade();
std::shared_ptr<OGLTransitionImpl> makeVenetianBlinds( bool vertical, int parts );
std::shared_ptr<OGLTransitionImpl> makeStatic();
std::shared_ptr<OGLTransitionImpl> makeDissolve();
std::shared_ptr<OGLTransitionImpl> makeVortex();
std::shared_ptr<OGLTransitionImpl> makeRipple();
std::shared_ptr<OGLTransitionImpl> makeGlitter();
std::shared_ptr<OGLTransitionImpl> makeHoneycomb();
std::shared_ptr<OGLTransitionImpl> makeNewsflash();

/** 2D replacements */

std::shared_ptr<OGLTransitionImpl> makeDiamond();
std::shared_ptr<OGLTransitionImpl> makeFadeSmoothly();
std::shared_ptr<OGLTransitionImpl> makeFadeThroughBlack();

class SceneObject : private boost::noncopyable
{
public:
    SceneObject();
    virtual ~SceneObject();

    virtual void prepare() {}
    virtual void display(double nTime, double SlideWidth, double SlideHeight, double DispWidth, double DispHeight) const;
    virtual void finish() {}

    void pushPrimitive (const Primitive &p);

protected:
    /** All the surrounding scene primitives
    */
    Primitives_t maPrimitives;
};

class Iris : public SceneObject
{
public:
    Iris ();

    virtual void prepare() override;
    virtual void display(double nTime, double SlideWidth, double SlideHeight, double DispWidth, double DispHeight) const override;
    virtual void finish() override;

private:
    GLuint maTexture;
};

/** This class is a list of Triangles that will share Operations, and could possibly share
*/
class Primitive
{
public:
    Primitive() {}
    // making copy constructor explicit makes the class un-suitable for use with stl containers
    Primitive(const Primitive& rvalue);
    Primitive& operator=(const Primitive& rvalue);

    void swap(Primitive& rOther);

    void applyOperations(double nTime, double SlideWidthScale, double SlideHeightScale) const;
    void display(double nTime, double SlideWidthScale, double SlideHeightScale) const;

    /** PushBack a vertex,normal, and tex coord. Each SlideLocation is where on the slide is mapped to this location ( from (0,0) to (1,1)  ). This will make sure the correct aspect ratio is used, and helps to make slides begin and end at the correct position. (0,0) is the top left of the slide, and (1,1) is the bottom right.

    @param SlideLocation0
    Location of first Vertex on slide

    @param SlideLocation1
    Location of second Vertex on slide

    @param SlideLocation2
    Location of third Vertex on slide

    */
    void pushTriangle(const glm::vec2& SlideLocation0,const glm::vec2& SlideLocation1,const glm::vec2& SlideLocation2);

    /** guards against directly changing the vertices

        @return
        the list of vertices
    */
    const std::vector<glm::vec3>& getVertices() const {return Vertices;}

    /** list of Operations to be performed on this primitive.These operations will be called in the order they were pushed back in. In OpenGL this effectively uses the operations in the opposite order they were pushed back.

        @return
        the list of Operations

    */
    Operations_t Operations;

private:
    /** list of vertices
    */
    std::vector<glm::vec3> Vertices;

    /** list of Normals
    */
    std::vector<glm::vec3> Normals;

    /** list of Texture Coordinates
    */
    std::vector<glm::vec2> TexCoords;
};

/** This class is to be derived to make any operation (transform) you may need in order to construct your transitions
*/
class Operation : private boost::noncopyable
{
public:
    virtual ~Operation(){}

protected:
    /** Should this operation be interpolated . If TRUE, the transform will smoothly move from making no difference from t = 0.0 to mnT0 to being completely transformed from t = mnT1 to 1. If FALSE, the transform will be inneffectual from t = 0 to mnT0, and completely transformed from t = mnT0 to 1.
    */
    bool mbInterpolate;

    /** time to begin the transformation
    */
    double mnT0;

    /** time to finish the transformation
    */
    double mnT1;
public:
    /** this is the function that is called to give the Operation to OpenGL.

        @param t
        time from t = 0 to t = 1

        @param SlideWidthScale
        width of slide divided by width of window

        @param SlideHeightScale
        height of slide divided by height of window

    */
    virtual void interpolate(double t,double SlideWidthScale,double SlideHeightScale) const = 0;

protected:
    Operation(bool bInterpolate, double nT0, double nT1):
        mbInterpolate(bInterpolate), mnT0(nT0), mnT1(nT1){}
};

/** this class is a generic CounterClockWise(CCW) rotation with an axis angle
*/
class SRotate: public Operation
{
public:
    virtual void interpolate(double t,double SlideWidthScale,double SlideHeightScale) const override;

    /** Constructor

        @param Axis
        axis to rotate about

        @param Origin
        position that rotation axis runs through

        @param Angle
        angle in radians of CCW rotation

        @param bInter
        see Operation

        @param T0
        transformation starting time

        @param T1
        transformation ending time

    */
    SRotate(const glm::vec3& Axis, const glm::vec3& Origin, double Angle,
            bool bInter, double T0, double T1);
    virtual ~SRotate(){}
private:
    /** axis to rotate CCW about
    */
    glm::vec3 axis;

    /** position that rotation axis runs through
    */
    glm::vec3 origin;

    /** angle in radians of CCW rotation
    */
    double angle;
};

std::shared_ptr<SRotate>
makeSRotate(const glm::vec3& Axis, const glm::vec3& Origin, double Angle,
        bool bInter, double T0, double T1);

/** scaling transformation
*/
class SScale: public Operation
{
public:
    virtual void interpolate(double t,double SlideWidthScale,double SlideHeightScale) const override;

    /** Constructor

        @param Scale
        amount to scale by

        @param Origin
        position that rotation axis runs through

        @param bInter
        see Operation

        @param T0
        transformation starting time

        @param T1
        transformation ending time

    */
    SScale(const glm::vec3& Scale, const glm::vec3& Origin,bool bInter, double T0, double T1);
    virtual ~SScale(){}
private:
    glm::vec3 scale;
    glm::vec3 origin;
};

std::shared_ptr<SScale>
makeSScale(const glm::vec3& Scale, const glm::vec3& Origin,bool bInter, double T0, double T1);

/** translation transformation
*/
class STranslate: public Operation
{
public:
    virtual void interpolate(double t,double SlideWidthScale,double SlideHeightScale) const override;

    /** Constructor

        @param Vector
        vector to translate

        @param bInter
        see Operation

        @param T0
        transformation starting time

        @param T1
        transformation ending time

    */
    STranslate(const glm::vec3& Vector,bool bInter, double T0, double T1);
    virtual ~STranslate(){}
private:
    /** vector to translate by
    */
    glm::vec3 vector;
};

std::shared_ptr<STranslate>
makeSTranslate(const glm::vec3& Vector,bool bInter, double T0, double T1);

/** translation transformation
*/
class SEllipseTranslate: public Operation
{
public:
    virtual void interpolate(double t,double SlideWidthScale,double SlideHeightScale) const override;

    /** Constructor

        @param Vector
        vector to translate

        @param bInter
        see Operation

        @param T0
        transformation starting time

        @param T1
        transformation ending time

    */
    SEllipseTranslate(double dWidth, double dHeight, double dStartPosition, double dEndPosition, bool bInter, double T0, double T1);
    virtual ~SEllipseTranslate(){}
private:
    /** width and length of the ellipse
     */
    double width, height;

    /** start and end position on the ellipse <0,1>
     */
    double startPosition;
    double endPosition;
};

std::shared_ptr<SEllipseTranslate>
makeSEllipseTranslate(double dWidth, double dHeight, double dStartPosition, double dEndPosition, bool bInter, double T0, double T1);

/** Same as SRotate, except the depth is scaled by the width of the slide divided by the width of the window.
*/
class RotateAndScaleDepthByWidth: public Operation
{
public:
    virtual void interpolate(double t,double SlideWidthScale,double SlideHeightScale) const override;

    RotateAndScaleDepthByWidth(const glm::vec3& Axis,const glm::vec3& Origin,double Angle,bool bInter, double T0, double T1);
    virtual ~RotateAndScaleDepthByWidth(){}
private:
    glm::vec3 axis;
    glm::vec3 origin;
    double angle;
};

std::shared_ptr<RotateAndScaleDepthByWidth>
makeRotateAndScaleDepthByWidth(const glm::vec3& Axis,const glm::vec3& Origin,double Angle,bool bInter, double T0, double T1);

/** Same as SRotate, except the depth is scaled by the width of the slide divided by the height of the window.
*/
class RotateAndScaleDepthByHeight: public Operation
{
public:
    virtual void interpolate(double t,double SlideWidthScale,double SlideHeightScale) const override;

    RotateAndScaleDepthByHeight(const glm::vec3& Axis,const glm::vec3& Origin,double Angle,bool bInter, double T0, double T1);
    virtual ~RotateAndScaleDepthByHeight(){}
private:
    glm::vec3 axis;
    glm::vec3 origin;
    double angle;
};

std::shared_ptr<RotateAndScaleDepthByHeight>
makeRotateAndScaleDepthByHeight(const glm::vec3& Axis,const glm::vec3& Origin,double Angle,bool bInter, double T0, double T1);

#endif // INCLUDED_SLIDESHOW_TRANSITION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
