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

#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/vector/b3dvector.hxx>

#if defined( WNT )
#elif defined( OS2 )
#elif defined( QUARTZ )
#include <OpenGL/gl.h>
#elif defined( UNX ) && !defined( QUARTZ )
#endif

#include <vector>

#if !defined( QUARTZ )
#include <GL/gl.h>
#endif

using namespace std;

class Primitive;
class Operation;
class SceneObject;


/** OpenGL 3D Transition class. It implicitly is constructed from XOGLTransition

    This class is capable of making itself into many difference transitions. It holds Primitives and Operations on those primitives.
*/
class OGLTransitionImpl
{
public:
    OGLTransitionImpl() :
        mbUseMipMapLeaving( true ),
        mbUseMipMapEntering( true ),
        mnRequiredGLVersion( 1.0 ),
        maLeavingSlidePrimitives(),
        maEnteringSlidePrimitives(),
        maSceneObjects(),
        mbReflectSlides( false ),
        mVertexObject( 0 ),
        mFragmentObject( 0 ),
        mProgramObject( 0 ),
        maHelperTexture( 0 ),
        mmPrepare( NULL ),
        mmPrepareTransition( NULL ),
        mmClearTransition( NULL ),
        mmDisplaySlides( NULL )
    {}

    ~OGLTransitionImpl();

    void prepare( ::sal_Int32 glLeavingSlideTex, ::sal_Int32 glEnteringSlideTex );
    void display( double nTime, ::sal_Int32 glLeavingSlideTex, ::sal_Int32 glEnteringSlideTex, double SlideWidth, double SlideHeight, double DispWidth, double DispHeight );
    void finish();

    void makeOutsideCubeFaceToLeft();
    void makeInsideCubeFaceToLeft();
    void makeNByMTileFlip( ::sal_uInt16 n, ::sal_uInt16 m );
    void makeRevolvingCircles( ::sal_uInt16 nCircles , ::sal_uInt16 nPointsOnCircles );
    void makeHelix( ::sal_uInt16 nRows );
    void makeFallLeaving();
    void makeTurnAround();
    void makeTurnDown();
    void makeIris();
    void makeRochade();
    void makeVenetianBlinds( bool vertical, int parts );
    void makeStatic();
    void makeDissolve();
    void makeNewsflash();

    /** 2D replacements
     */
    void makeDiamond();
    void makeFadeSmoothly();
    void makeFadeThroughBlack();

    /** Whether to use mipmaping for slides textures
     */
    bool mbUseMipMapLeaving;
    bool mbUseMipMapEntering;

    /** which GL version does the transition require
     */
    float mnRequiredGLVersion;

private:
    /** clears all the primitives and operations
    */
    void clear();

    /** All the primitives that use the leaving slide texture
    */
    vector<Primitive> maLeavingSlidePrimitives;

    /** All the primitives that use the leaving slide texture
    */
    vector<Primitive> maEnteringSlidePrimitives;

    /** All the surrounding scene objects
    */
    vector<SceneObject*> maSceneObjects;

    /** All the operations that should be applied to both leaving and entering slide primitives. These operations will be called in the order they were pushed back in. In OpenGL this effectively uses the operations in the opposite order they were pushed back.
    */
    vector<Operation*> OverallOperations;

    /** Whether to reflect slides, the reflection happens on flat surface beneath the slides.
     ** Now it only works with slides which keep their rectangular shape together.
     */
    bool mbReflectSlides;

    /** GLSL objects, shaders and program
     */
    GLuint mVertexObject, mFragmentObject, mProgramObject;

    /** various data */
    GLuint maHelperTexture;

    /** When this method is not NULL, it is called in display method to prepare the slides, scene, etc.
     ** We might later replace this by cleaner derived class.
     */
    void (OGLTransitionImpl::*mmPrepare)( double nTime, double SlideWidth, double SlideHeight, double DispWidth, double DispHeight );

    /** When this method is not NULL, it is called after glx context is ready to let the transition prepare GL related things, like GLSL program.
     ** We might later replace this by cleaner derived class.
     */
    void (OGLTransitionImpl::*mmPrepareTransition)( ::sal_Int32 glLeavingSlideTex, ::sal_Int32 glEnteringSlideTex );

    /** When this method is not NULL, it is called when the transition needs to clear after itself, like delete own textures etc.
     ** We might later replace this by cleaner derived class.
     */
    void (OGLTransitionImpl::*mmClearTransition)();

    /** When this method is not NULL, it is called in display method to display the slides.
     ** We might later replace this by cleaner derived class.
     */
    void (OGLTransitionImpl::*mmDisplaySlides)( double nTime, ::sal_Int32 glLeavingSlideTex, ::sal_Int32 glEnteringSlideTex, double SlideWidthScale, double SlideHeightScale );

    void displaySlides( double nTime, ::sal_Int32 glLeavingSlideTex, ::sal_Int32 glEnteringSlideTex, double SlideWidthScale, double SlideHeightScale );
    void displaySlide( double nTime, ::sal_Int32 glSlideTex, std::vector<Primitive>& primitives, double SlideWidthScale, double SlideHeightScale );
    void displayScene( double nTime, double SlideWidth, double SlideHeight, double DispWidth, double DispHeight);
    void applyOverallOperations( double nTime, double SlideWidthScale, double SlideHeightScale );

    /** various transitions helper methods
     */
    void prepareDiamond( double nTime, double SlideWidth, double SlideHeight,double DispWidth, double DispHeight );
    void displaySlidesFadeSmoothly( double nTime, ::sal_Int32 glLeavingSlideTex, ::sal_Int32 glEnteringSlideTex, double SlideWidthScale, double SlideHeightScale );
        void displaySlidesFadeThroughBlack( double nTime, ::sal_Int32 glLeavingSlideTex, ::sal_Int32 glEnteringSlideTex, double SlideWidthScale, double SlideHeightScale );
        void displaySlidesRochade( double nTime, ::sal_Int32 glLeavingSlideTex, ::sal_Int32 glEnteringSlideTex, double SlideWidthScale, double SlideHeightScale );
    void displaySlidesShaders( double nTime, ::sal_Int32 glLeavingSlideTex, ::sal_Int32 glEnteringSlideTex, double SlideWidthScale, double SlideHeightScale );
    void prepareStatic( ::sal_Int32 glLeavingSlideTex, ::sal_Int32 glEnteringSlideTex );
    void prepareDissolve( ::sal_Int32 glLeavingSlideTex, ::sal_Int32 glEnteringSlideTex );
    void preparePermShader();
};

class SceneObject
{
public:
    SceneObject();

    virtual void prepare() {};
    virtual void display(double nTime, double SlideWidth, double SlideHeight, double DispWidth, double DispHeight);
    virtual void finish() {};

    void pushPrimitive (const Primitive &p);

protected:
    /** All the surrounding scene primitives
    */
    vector<Primitive> maPrimitives;
};

class Iris : public SceneObject
{
public:
    Iris ();

    virtual void prepare();
    virtual void display(double nTime, double SlideWidth, double SlideHeight, double DispWidth, double DispHeight);
    virtual void finish();

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
    ~Primitive();

    void applyOperations(double nTime, double SlideWidthScale, double SlideHeightScale);
    void display(double nTime, double SlideWidthScale, double SlideHeightScale);
    const Primitive& operator=(const Primitive& rvalue);

    /** PushBack a vertex,normal, and tex coord. Each SlideLocation is where on the slide is mapped to this location ( from (0,0) to (1,1)  ). This will make sure the correct aspect ratio is used, and helps to make slides begin and end at the correct position. (0,0) is the top left of the slide, and (1,1) is the bottom right.

    @param SlideLocation0
    Location of first Vertex on slide

    @param SlideLocation1
    Location of second Vertex on slide

    @param SlideLocation2
    Location of third Vertex on slide

    */
    void pushTriangle(const basegfx::B2DVector& SlideLocation0,const basegfx::B2DVector& SlideLocation1,const basegfx::B2DVector& SlideLocation2);

    /** clear all the vertices, normals, tex coordinates, and normals
    */
    void clearTriangles();

    /** guards against directly changing the vertices

        @return
        the list of vertices
    */
    const vector<basegfx::B3DVector>& getVertices() const {return Vertices;}

    /** guards against directly changing the vertices
    */
    const vector<basegfx::B3DVector>& getNormals() const {return Normals;}

    /** guards against directly changing the vertices

        @return
        the list of Texture Coordinates

    */
    const vector<basegfx::B2DVector>& getTexCoords() const {return TexCoords;}

    /** list of Operations to be performed on this primitive.These operations will be called in the order they were pushed back in. In OpenGL this effectively uses the operations in the opposite order they were pushed back.

        @return
        the list of Operations

    */
    vector<Operation*> Operations;

private:
    /** list of vertices
    */
    vector<basegfx::B3DVector> Vertices;

    /** list of Normals
    */
    vector<basegfx::B3DVector> Normals;

    /** list of Texture Coordinates
    */
    vector<basegfx::B2DVector> TexCoords;
};

/** This class is to be derived to make any operation (tranform) you may need in order to construct your transitions
*/
class Operation
{
public:
    Operation(){}
    virtual ~Operation(){}

    /** Should this operation be interpolated . If TRUE, the transform will smoothly move from making no difference from t = 0.0 to nT0 to being completely transformed from t = nT1 to 1. If FALSE, the transform will be inneffectual from t = 0 to nT0, and completely transformed from t = nT0 to 1.
    */
    bool bInterpolate;

    /** time to begin the transformation
    */
    double nT0;

    /** time to finish the transformation
    */
    double nT1;
public:
    /** this is the function that is called to give the Operation to OpenGL.

        @param t
        time from t = 0 to t = 1

        @param SlideWidthScale
        width of slide divided by width of window

        @param SlideHeightScale
        height of slide divided by height of window

    */
    virtual void interpolate(double t,double SlideWidthScale,double SlideHeightScale) = 0;

    /** return a copy of this operation
    */
    virtual Operation* clone() = 0;
};

/** this class is a generic CounterClockWise(CCW) rotation with an axis angle
*/
class SRotate: public Operation
{
public:
    void interpolate(double t,double SlideWidthScale,double SlideHeightScale);
    virtual SRotate* clone();

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
    SRotate(const basegfx::B3DVector& Axis,const basegfx::B3DVector& Origin,double Angle,bool bInter, double T0, double T1);
    ~SRotate(){}
private:
    /** axis to rotate CCW about
    */
    basegfx::B3DVector axis;

    /** position that rotation axis runs through
    */
    basegfx::B3DVector origin;

    /** angle in radians of CCW rotation
    */
    double angle;
};

/** scaling transformation
*/
class SScale: public Operation
{
public:
    void interpolate(double t,double SlideWidthScale,double SlideHeightScale);
    SScale* clone();

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
    SScale(const basegfx::B3DVector& Scale, const basegfx::B3DVector& Origin,bool bInter, double T0, double T1);
    ~SScale(){}
private:
    basegfx::B3DVector scale;
    basegfx::B3DVector origin;
};

/** translation transformation
*/
class STranslate: public Operation
{
public:
    void interpolate(double t,double SlideWidthScale,double SlideHeightScale);
    STranslate* clone();

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
    STranslate(const basegfx::B3DVector& Vector,bool bInter, double T0, double T1);
    ~STranslate(){}
private:
    /** vector to translate by
    */
    basegfx::B3DVector vector;
};

/** translation transformation
*/
class SEllipseTranslate: public Operation
{
public:
    void interpolate(double t,double SlideWidthScale,double SlideHeightScale);
    SEllipseTranslate* clone();

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
    ~SEllipseTranslate(){}
private:
    /** width and length of the ellipse
     */
    double width, height;

    /** start and end position on the ellipse <0,1>
     */
    double startPosition;
    double endPosition;
};

/** Same as SRotate, except the depth is scaled by the width of the slide divided by the width of the window.
*/
class RotateAndScaleDepthByWidth: public Operation
{
public:
    void interpolate(double t,double SlideWidthScale,double SlideHeightScale);
    RotateAndScaleDepthByWidth* clone();

    RotateAndScaleDepthByWidth(const basegfx::B3DVector& Axis,const basegfx::B3DVector& Origin,double Angle,bool bInter, double T0, double T1);
    ~RotateAndScaleDepthByWidth(){}
private:
    basegfx::B3DVector axis;
    basegfx::B3DVector origin;
    double angle;
};

/** Same as SRotate, except the depth is scaled by the width of the slide divided by the height of the window.
*/
class RotateAndScaleDepthByHeight: public Operation
{
public:
    void interpolate(double t,double SlideWidthScale,double SlideHeightScale);
    RotateAndScaleDepthByHeight* clone();

    RotateAndScaleDepthByHeight(const basegfx::B3DVector& Axis,const basegfx::B3DVector& Origin,double Angle,bool bInter, double T0, double T1);
    ~RotateAndScaleDepthByHeight(){}
private:
    basegfx::B3DVector axis;
    basegfx::B3DVector origin;
    double angle;
};

#endif // INCLUDED_SLIDESHOW_TRANSITION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
