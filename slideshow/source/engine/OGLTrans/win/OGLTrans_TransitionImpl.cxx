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

#include "OGLTrans_TransitionImpl.hxx"
#include "OGLTrans_Shaders.hxx"
#ifdef MACOSX
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#include <math.h>
#endif

void OGLTransitionImpl::clear()
{
    for(unsigned int i( 0 ); i < OverallOperations.size(); ++i)
        delete OverallOperations[i];
    OverallOperations.clear();
    maLeavingSlidePrimitives.clear();
    maEnteringSlidePrimitives.clear();
    for(unsigned int i(0); i < maSceneObjects.size(); ++i)
        delete maSceneObjects[i];
    maSceneObjects.clear();

    mbReflectSlides = false;

#ifdef GL_VERSION_2_0
    if( mProgramObject ) {
        OGLShaders::glDeleteProgram( mProgramObject );
        mProgramObject = 0;
    }

    if( mVertexObject ) {
        OGLShaders::glDeleteShader( mVertexObject );
        mVertexObject = 0;
    }

    if( mFragmentObject ) {
        OGLShaders::glDeleteShader( mFragmentObject );
        mFragmentObject = 0;
    }
#endif

    if( maHelperTexture ) {
        glDeleteTextures( 1, &maHelperTexture );
        maHelperTexture = 0;
    }

    if( mmClearTransition )
        (this->*mmClearTransition)();
}

OGLTransitionImpl::~OGLTransitionImpl()
{
    clear();
}

void OGLTransitionImpl::prepare( ::sal_Int32 glLeavingSlideTex, ::sal_Int32 glEnteringSlideTex )
{
    for(unsigned int i(0); i < maSceneObjects.size(); ++i) {
        maSceneObjects[i]->prepare();
    }

    if( mmPrepareTransition )
        (this->*mmPrepareTransition)( glLeavingSlideTex, glEnteringSlideTex );
}

void OGLTransitionImpl::finish()
{
    for(unsigned int i(0); i < maSceneObjects.size(); ++i) {
        maSceneObjects[i]->finish();
    }
}

static void blendSlide( double depth )
{
    double showHeight = -1 + depth*2;
    GLfloat reflectionColor[] = {0, 0, 0, 0.25};

    glDisable( GL_DEPTH_TEST );
    glBegin( GL_QUADS );
    glColor4fv( reflectionColor );
    glVertex3f( -1, -1, 0 );
    glColor4f( 0, 0, 0, 1 );
    glVertex3f(-1,  showHeight, 0 );
    glVertex3f( 1,  showHeight, 0 );
    glColor4fv( reflectionColor );
    glVertex3f( 1, -1, 0 );
    glEnd();

    glBegin( GL_QUADS );
    glColor4f( 0, 0, 0, 1 );
    glVertex3f( -1, showHeight, 0 );
    glVertex3f( -1,  1, 0 );
    glVertex3f(  1,  1, 0 );
    glVertex3f(  1, showHeight, 0 );
    glEnd();
    glEnable( GL_DEPTH_TEST );
}

static void slideShadow( double nTime, Primitive& primitive, double sw, double sh )
{
    double reflectionDepth = 0.3;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_LIGHTING);

    glPushMatrix();
    primitive.applyOperations( nTime, sw, sh );
    blendSlide( reflectionDepth );
    glPopMatrix();

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

void OGLTransitionImpl::display( double nTime, ::sal_Int32 glLeavingSlideTex, ::sal_Int32 glEnteringSlideTex,
                                 double SlideWidth, double SlideHeight, double DispWidth, double DispHeight )
{
    double SlideWidthScale, SlideHeightScale;

    SlideWidthScale = SlideWidth/DispWidth;
    SlideHeightScale = SlideHeight/DispHeight;

    if( mmPrepare ) {
        clear();
        (this->*mmPrepare)( nTime, SlideWidth, SlideHeight, DispWidth, DispHeight );
    }

    glPushMatrix();
    displaySlides( nTime, glLeavingSlideTex, glEnteringSlideTex, SlideWidthScale, SlideHeightScale );
    displayScene( nTime, SlideWidth, SlideHeight, DispWidth, DispHeight );
    glPopMatrix();
}

void OGLTransitionImpl::applyOverallOperations( double nTime, double SlideWidthScale, double SlideHeightScale )
{
    for(unsigned int i(0); i < OverallOperations.size(); ++i)
        OverallOperations[i]->interpolate(nTime,SlideWidthScale,SlideHeightScale);
}

void OGLTransitionImpl::displaySlide( double nTime, ::sal_Int32 glSlideTex, std::vector<Primitive>& primitives,
                                      double SlideWidthScale, double SlideHeightScale )
{
   //TODO change to foreach
    glBindTexture(GL_TEXTURE_2D, glSlideTex);

    // display slide reflection
    // note that depth test is turned off while blending the shadow
    // so the slides has to be rendered in right order, see rochade as example
    if( mbReflectSlides ) {
        double surfaceLevel = -0.04;

        /* reflected slides */
        glPushMatrix();

        glScaled( 1, -1, 1 );
        glTranslated( 0, 2 - surfaceLevel, 0 );

        glCullFace(GL_FRONT);
    for(unsigned int i(0); i < primitives.size(); ++i)
        primitives[i].display(nTime, SlideWidthScale, SlideHeightScale);
        glCullFace(GL_BACK);

    slideShadow( nTime, primitives[0], SlideWidthScale, SlideHeightScale );

        glPopMatrix();
    }

    for(unsigned int i(0); i < primitives.size(); ++i)
        primitives[i].display(nTime, SlideWidthScale, SlideHeightScale);
}

void OGLTransitionImpl::displaySlides( double nTime, ::sal_Int32 glLeavingSlideTex, ::sal_Int32 glEnteringSlideTex,
                                       double SlideWidthScale, double SlideHeightScale )
{
    if( mmDisplaySlides )
        (this->*mmDisplaySlides)( nTime, glLeavingSlideTex, glEnteringSlideTex, SlideWidthScale, SlideHeightScale );
    else {
        applyOverallOperations( nTime, SlideWidthScale, SlideHeightScale );

        glEnable(GL_TEXTURE_2D);
        displaySlide( nTime, glLeavingSlideTex, maLeavingSlidePrimitives, SlideWidthScale, SlideHeightScale );
        displaySlide( nTime, glEnteringSlideTex, maEnteringSlidePrimitives, SlideWidthScale, SlideHeightScale );
    }
}

void OGLTransitionImpl::displayScene( double nTime, double SlideWidth, double SlideHeight, double DispWidth, double DispHeight )
{
    glEnable(GL_TEXTURE_2D);
    for(unsigned int i(0); i < maSceneObjects.size(); ++i)
        maSceneObjects[i]->display(nTime, SlideWidth, SlideHeight, DispWidth, DispHeight);
}

void Primitive::display(double nTime, double WidthScale, double HeightScale)
{
    glPushMatrix();

    applyOperations( nTime, WidthScale, HeightScale );

    glEnableClientState( GL_VERTEX_ARRAY );
    if(!Normals.empty())
    {
        glNormalPointer( GL_DOUBLE , 0 , &Normals[0] );
        glEnableClientState( GL_NORMAL_ARRAY );
    }
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );
    glTexCoordPointer( 2, GL_DOUBLE, 0, &TexCoords[0] );
    glVertexPointer( 3, GL_DOUBLE, 0, &Vertices[0] );
    glDrawArrays( GL_TRIANGLES, 0, Vertices.size() );
    glPopMatrix();
}

void Primitive::applyOperations(double nTime, double WidthScale, double HeightScale)
{
    for(unsigned int i(0); i < Operations.size(); ++i)
        Operations[i]->interpolate( nTime ,WidthScale,HeightScale);
    glScaled(WidthScale,HeightScale,1);
}

Primitive::~Primitive()
{
    for(unsigned int i( 0 ); i < Operations.size(); ++i)
        delete Operations[i];
}


void SceneObject::display(double nTime, double /* SlideWidth */, double /* SlideHeight */, double DispWidth, double DispHeight )
{
    for(unsigned int i(0); i < maPrimitives.size(); ++i) {
        // fixme: allow various model spaces, now we make it so that
        // it is regular -1,-1 to 1,1, where the whole display fits in
        glPushMatrix();
        if (DispHeight > DispWidth)
            glScaled(DispHeight/DispWidth, 1, 1);
        else
            glScaled(1, DispWidth/DispHeight, 1);
        maPrimitives[i].display(nTime, 1, 1);
        glPopMatrix();
    }
}

void SceneObject::pushPrimitive(const Primitive &p)
{
    maPrimitives.push_back(p);
}

SceneObject::SceneObject()
    : maPrimitives()
{
}

Iris::Iris()
    : SceneObject ()
{
}

void Iris::display(double nTime, double SlideWidth, double SlideHeight, double DispWidth, double DispHeight )
{
    glBindTexture(GL_TEXTURE_2D, maTexture);
    SceneObject::display(nTime, SlideWidth, SlideHeight, DispWidth, DispHeight);
}

void Iris::prepare()
{
    static const GLubyte img[3] = { 80, 80, 80 };

    glGenTextures(1, &maTexture);
    glBindTexture(GL_TEXTURE_2D, maTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
}

void Iris::finish()
{
    glDeleteTextures(1, &maTexture);
}

void OGLTransitionImpl::makeOutsideCubeFaceToLeft()
{
    clear();
    Primitive Slide;

    Slide.pushTriangle(basegfx::B2DVector(0,0),basegfx::B2DVector(1,0),basegfx::B2DVector(0,1));
    Slide.pushTriangle(basegfx::B2DVector(1,0),basegfx::B2DVector(0,1),basegfx::B2DVector(1,1));

    maLeavingSlidePrimitives.push_back(Slide);

    Slide.Operations.push_back(new RotateAndScaleDepthByWidth(basegfx::B3DVector(0,1,0),basegfx::B3DVector(0,0,-1),90,false,0.0,1.0));

    maEnteringSlidePrimitives.push_back(Slide);

    OverallOperations.push_back(new RotateAndScaleDepthByWidth(basegfx::B3DVector(0,1,0),basegfx::B3DVector(0,0,-1),-90,true,0.0,1.0));
}

void OGLTransitionImpl::makeInsideCubeFaceToLeft()
{
    clear();
    Primitive Slide;

    Slide.pushTriangle(basegfx::B2DVector(0,0),basegfx::B2DVector(1,0),basegfx::B2DVector(0,1));
    Slide.pushTriangle(basegfx::B2DVector(1,0),basegfx::B2DVector(0,1),basegfx::B2DVector(1,1));

    maLeavingSlidePrimitives.push_back(Slide);

    Slide.Operations.push_back(new RotateAndScaleDepthByWidth(basegfx::B3DVector(0,1,0),basegfx::B3DVector(0,0,1),-90,false,0.0,1.0));

    maEnteringSlidePrimitives.push_back(Slide);

    OverallOperations.push_back(new RotateAndScaleDepthByWidth(basegfx::B3DVector(0,1,0),basegfx::B3DVector(0,0,1),90,true,0.0,1.0));
}

void OGLTransitionImpl::makeFallLeaving()
{
    clear();
    Primitive Slide;

    Slide.pushTriangle(basegfx::B2DVector(0,0),basegfx::B2DVector(1,0),basegfx::B2DVector(0,1));
    Slide.pushTriangle(basegfx::B2DVector(1,0),basegfx::B2DVector(0,1),basegfx::B2DVector(1,1));
    maEnteringSlidePrimitives.push_back(Slide);

    Slide.Operations.push_back(new RotateAndScaleDepthByWidth(basegfx::B3DVector(1,0,0),basegfx::B3DVector(0,-1,0), 90,true,0.0,1.0));
    maLeavingSlidePrimitives.push_back(Slide);

    mbUseMipMapEntering = false;
}

void OGLTransitionImpl::makeTurnAround()
{
    clear();
    Primitive Slide;

    mbReflectSlides = true;

    Slide.pushTriangle(basegfx::B2DVector(0,0),basegfx::B2DVector(1,0),basegfx::B2DVector(0,1));
    Slide.pushTriangle(basegfx::B2DVector(1,0),basegfx::B2DVector(0,1),basegfx::B2DVector(1,1));
    maLeavingSlidePrimitives.push_back(Slide);

    Slide.Operations.push_back(new RotateAndScaleDepthByWidth(basegfx::B3DVector(0,1,0),basegfx::B3DVector(0,0,0),-180,false,0.0,1.0));
    maEnteringSlidePrimitives.push_back(Slide);

    OverallOperations.push_back(new STranslate(basegfx::B3DVector(0, 0, -1.5),true, 0, 0.5));
    OverallOperations.push_back(new STranslate(basegfx::B3DVector(0, 0, 1.5), true, 0.5, 1));
    OverallOperations.push_back(new RotateAndScaleDepthByWidth(basegfx::B3DVector(0, 1, 0),basegfx::B3DVector(0, 0, 0), -180, true, 0.0, 1.0));
}

void OGLTransitionImpl::makeTurnDown()
{
    clear();
    Primitive Slide;

    Slide.pushTriangle(basegfx::B2DVector(0,0),basegfx::B2DVector(1,0),basegfx::B2DVector(0,1));
    Slide.pushTriangle(basegfx::B2DVector(1,0),basegfx::B2DVector(0,1),basegfx::B2DVector(1,1));
    maLeavingSlidePrimitives.push_back(Slide);

    Slide.Operations.push_back(new STranslate(basegfx::B3DVector(0, 0, 0.0001), false, -1.0, 0.0));
    Slide.Operations.push_back(new SRotate (basegfx::B3DVector(0, 0, 1), basegfx::B3DVector(-1, 1, 0), -90, true, 0.0, 1.0));
    Slide.Operations.push_back(new SRotate (basegfx::B3DVector(0, 0, 1), basegfx::B3DVector(-1, 1, 0), 90, false, -1.0, 0.0));
    maEnteringSlidePrimitives.push_back(Slide);

    mbUseMipMapLeaving = false;
}

void OGLTransitionImpl::makeIris()
{
    clear();
    Primitive Slide;

    Slide.pushTriangle (basegfx::B2DVector (0,0), basegfx::B2DVector (1,0), basegfx::B2DVector (0,1));
    Slide.pushTriangle (basegfx::B2DVector (1,0), basegfx::B2DVector (0,1), basegfx::B2DVector (1,1));
    maEnteringSlidePrimitives.push_back (Slide);

    Slide.Operations.push_back (new STranslate (basegfx::B3DVector (0, 0,  0.000001), false, -1, 0));
    Slide.Operations.push_back (new STranslate (basegfx::B3DVector (0, 0, -0.000002), false, 0.5, 1));
    maLeavingSlidePrimitives.push_back (Slide);


    Primitive irisPart, part;
    int i, nSteps = 24, nParts = 7;
    double t = 1.0/nSteps, cx, cy, lcx, lcy, lx = 1, ly = 0, x, y, cxo, cyo, lcxo, lcyo, of=2.2, f=1.42;

    for (i=1; i<=nSteps; i++) {
        x = cos ((3*2*M_PI*t)/nParts);
        y = -sin ((3*2*M_PI*t)/nParts);
        cx = (f*x + 1)/2;
        cy = (f*y + 1)/2;
        lcx = (f*lx + 1)/2;
        lcy = (f*ly + 1)/2;
        cxo = (of*x + 1)/2;
        cyo = (of*y + 1)/2;
        lcxo = (of*lx + 1)/2;
        lcyo = (of*ly + 1)/2;
        irisPart.pushTriangle (basegfx::B2DVector (lcx, lcy),
                               basegfx::B2DVector (lcxo, lcyo),
                               basegfx::B2DVector (cx, cy));
        irisPart.pushTriangle (basegfx::B2DVector (cx, cy),
                               basegfx::B2DVector (lcxo, lcyo),
                               basegfx::B2DVector (cxo, cyo));
        lx = x;
        ly = y;
        t += 1.0/nSteps;
    }

    Iris* pIris = new Iris();
    double angle = 87;

    for (i = 0; i < nParts; i++) {
        irisPart.Operations.clear ();
        double rx, ry;

        rx = cos ((2*M_PI*i)/nParts);
        ry = sin ((2*M_PI*i)/nParts);
        irisPart.Operations.push_back (new SRotate (basegfx::B3DVector(0, 0, 1), basegfx::B3DVector(rx, ry, 0),  angle, true, 0.0, 0.5));
        irisPart.Operations.push_back (new SRotate (basegfx::B3DVector(0, 0, 1), basegfx::B3DVector(rx, ry, 0), -angle, true, 0.5, 1));
        if (i > 0) {
            irisPart.Operations.push_back (new STranslate (basegfx::B3DVector(rx, ry, 0),  false, -1, 0));
            irisPart.Operations.push_back (new SRotate (basegfx::B3DVector(0, 0, 1), basegfx::B3DVector(0, 0, 0), i*360.0/nParts, false, -1, 0));
            irisPart.Operations.push_back (new STranslate (basegfx::B3DVector(-1, 0, 0),  false, -1, 0));
        }
        irisPart.Operations.push_back(new STranslate(basegfx::B3DVector(0, 0, 1), false, -2, 0.0));
        irisPart.Operations.push_back (new SRotate (basegfx::B3DVector(1, .5, 0), basegfx::B3DVector(1, 0, 0), -30, false, -1, 0));
        pIris->pushPrimitive (irisPart);
    }

    maSceneObjects.push_back (pIris);

    mbUseMipMapLeaving = mbUseMipMapEntering = false;
}

void OGLTransitionImpl::displaySlidesRochade( double nTime, ::sal_Int32 glLeavingSlideTex, ::sal_Int32 glEnteringSlideTex,
                          double SlideWidthScale, double SlideHeightScale )
{
    applyOverallOperations( nTime, SlideWidthScale, SlideHeightScale );

    glEnable(GL_TEXTURE_2D);

    if( nTime > .5) {
    displaySlide( nTime, glLeavingSlideTex, maLeavingSlidePrimitives, SlideWidthScale, SlideHeightScale );
    displaySlide( nTime, glEnteringSlideTex, maEnteringSlidePrimitives, SlideWidthScale, SlideHeightScale );
    } else {
    displaySlide( nTime, glEnteringSlideTex, maEnteringSlidePrimitives, SlideWidthScale, SlideHeightScale );
    displaySlide( nTime, glLeavingSlideTex, maLeavingSlidePrimitives, SlideWidthScale, SlideHeightScale );
    }
}

void OGLTransitionImpl::makeRochade()
{
    clear();
    Primitive Slide;

    mbReflectSlides = true;
    mmDisplaySlides = &OGLTransitionImpl::displaySlidesRochade;

    double w, h;

    w = 2.2;
    h = 10;

    Slide.pushTriangle(basegfx::B2DVector(0,0),basegfx::B2DVector(1,0),basegfx::B2DVector(0,1));
    Slide.pushTriangle(basegfx::B2DVector(1,0),basegfx::B2DVector(0,1),basegfx::B2DVector(1,1));

    Slide.Operations.push_back(new SEllipseTranslate(w, h, 0.25, -0.25, true, 0, 1));
    Slide.Operations.push_back(new RotateAndScaleDepthByWidth(basegfx::B3DVector(0,1,0),basegfx::B3DVector(0,0,0), -45, true, 0, 1));
    maLeavingSlidePrimitives.push_back(Slide);

    Slide.Operations.clear();
    Slide.Operations.push_back(new SEllipseTranslate(w, h, 0.75, 0.25, true, 0, 1));
    Slide.Operations.push_back(new STranslate(basegfx::B3DVector(0, 0, -h), false, -1, 0));
    Slide.Operations.push_back(new RotateAndScaleDepthByWidth(basegfx::B3DVector(0,1,0),basegfx::B3DVector(0,0,0), -45, true, 0, 1));
    Slide.Operations.push_back(new RotateAndScaleDepthByWidth(basegfx::B3DVector(0,1,0),basegfx::B3DVector(0,0,0), 45, false, -1, 0));
    maEnteringSlidePrimitives.push_back(Slide);

}

// TODO(Q3): extract to basegfx
inline basegfx::B2DVector clamp(const basegfx::B2DVector& v)
{
    return basegfx::B2DVector(min(max(v.getX(),-1.0),1.0),
                              min(max(v.getY(),-1.0),1.0));
}

// TODO(Q3): extract to basegfx
inline basegfx::B3DVector clamp(const basegfx::B3DVector& v)
{
    return basegfx::B3DVector(min(max(v.getX(),-1.0),1.0),
                              min(max(v.getY(),-1.0),1.0),
                              min(max(v.getZ(),-1.0),1.0));
}

inline double randFromNeg1to1()
{
    return ( ( static_cast<double>( rand() ) / static_cast<double>( RAND_MAX ) ) * 2.0 ) - 1.0;
}

// TODO(Q3): extract to basegfx
inline basegfx::B3DVector randNormVectorInXYPlane()
{
    basegfx::B3DVector toReturn(randFromNeg1to1(),randFromNeg1to1(),0.0);
    return toReturn/toReturn.getLength();
}

void OGLTransitionImpl::makeRevolvingCircles( ::sal_uInt16 nCircles , ::sal_uInt16 nPointsOnCircles )
{
    clear();
    double dAngle(2*3.1415926/static_cast<double>( nPointsOnCircles ));
    if(nCircles < 2 || nPointsOnCircles < 4)
    {
        makeNByMTileFlip(1,1);
        return;
    }
    double Radius(1.0/static_cast<double>( nCircles ));
    double dRadius(Radius);
    double LastRadius(0.0);
    double NextRadius(2*Radius);

    /// now we know there is at least two circles
    /// the first will always be a full circle
    /// the last will always be the outer shell of the slide with a circle hole

    //add the full circle
    vector<basegfx::B2DVector> unScaledTexCoords;
    double TempAngle(0.0);
    for(unsigned int Point(0); Point < nPointsOnCircles; ++Point)
    {
        unScaledTexCoords.push_back( basegfx::B2DVector( cos(TempAngle - 3.1415926/2.0) , sin(TempAngle- 3.1415926/2.0) ) );

        TempAngle += dAngle;
    }

    {
        Primitive EnteringSlide;
        Primitive LeavingSlide;
        for(int Point(0); Point + 1 < nPointsOnCircles; ++Point)
        {
            EnteringSlide.pushTriangle( basegfx::B2DVector( 0.5 , 0.5 ) , Radius * unScaledTexCoords[ Point + 1 ] / 2.0 + basegfx::B2DVector( 0.5 , 0.5 ) , Radius * unScaledTexCoords[ Point ] / 2.0 + basegfx::B2DVector( 0.5 , 0.5 ) );
            LeavingSlide.pushTriangle( basegfx::B2DVector( 0.5 , 0.5 ) , Radius * unScaledTexCoords[ Point + 1 ] / 2.0 + basegfx::B2DVector( 0.5 , 0.5 ) , Radius * unScaledTexCoords[ Point ] / 2.0 + basegfx::B2DVector( 0.5, 0.5) );
        }
        EnteringSlide.pushTriangle( basegfx::B2DVector(0.5,0.5) , Radius * unScaledTexCoords[ 0 ] / 2.0 + basegfx::B2DVector( 0.5 , 0.5 ) , Radius * unScaledTexCoords[ nPointsOnCircles - 1 ] / 2.0 + basegfx::B2DVector( 0.5 , 0.5 ) );
        LeavingSlide.pushTriangle( basegfx::B2DVector(0.5,0.5) , Radius*unScaledTexCoords[0]/2.0 + basegfx::B2DVector(0.5,0.5) , Radius*unScaledTexCoords[nPointsOnCircles - 1]/2.0 + basegfx::B2DVector(0.5,0.5) );

        basegfx::B3DVector axis(randNormVectorInXYPlane());
        EnteringSlide.Operations.push_back( new SRotate( axis , basegfx::B3DVector(0,0,0) , 180, true, Radius/2.0 , (NextRadius + 1)/2.0 ) );
        LeavingSlide.Operations.push_back( new SRotate( axis , basegfx::B3DVector(0,0,0) , 180, true, Radius/2.0 , (NextRadius + 1)/2.0 ) );
        EnteringSlide.Operations.push_back( new SRotate( axis , basegfx::B3DVector(0,0,0) , -180, false,0.0,1.0) );

        maEnteringSlidePrimitives.push_back(EnteringSlide);
        maLeavingSlidePrimitives.push_back(LeavingSlide);
        LastRadius = Radius;
        Radius = NextRadius;
        NextRadius += dRadius;
    }

    for(int i(1); i < nCircles - 1; ++i)
    {
        Primitive LeavingSlide;
        Primitive EnteringSlide;
        for(int Side(0); Side < nPointsOnCircles - 1; ++Side)
        {
            EnteringSlide.pushTriangle(Radius*unScaledTexCoords[Side]/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[Side]/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[Side + 1]/2.0 + basegfx::B2DVector(0.5,0.5) );
            EnteringSlide.pushTriangle(Radius*unScaledTexCoords[Side]/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[Side + 1]/2.0 + basegfx::B2DVector(0.5,0.5) , Radius*unScaledTexCoords[Side + 1]/2.0 + basegfx::B2DVector(0.5,0.5) );

            LeavingSlide.pushTriangle(Radius*unScaledTexCoords[Side]/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[Side]/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[Side + 1]/2.0 + basegfx::B2DVector(0.5,0.5) );
            LeavingSlide.pushTriangle(Radius*unScaledTexCoords[Side]/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[Side + 1]/2.0 + basegfx::B2DVector(0.5,0.5) , Radius*unScaledTexCoords[Side + 1]/2.0 + basegfx::B2DVector(0.5,0.5) );
        }

        EnteringSlide.pushTriangle(Radius*unScaledTexCoords[nPointsOnCircles - 1]/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[nPointsOnCircles - 1]/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[0]/2.0 + basegfx::B2DVector(0.5,0.5) );
        EnteringSlide.pushTriangle(Radius*unScaledTexCoords[nPointsOnCircles - 1]/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[0]/2.0 + basegfx::B2DVector(0.5,0.5) , Radius*unScaledTexCoords[0]/2.0 + basegfx::B2DVector(0.5,0.5) );

        LeavingSlide.pushTriangle(Radius*unScaledTexCoords[nPointsOnCircles - 1]/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[nPointsOnCircles - 1]/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[0]/2.0 + basegfx::B2DVector(0.5,0.5) );
        LeavingSlide.pushTriangle(Radius*unScaledTexCoords[nPointsOnCircles - 1]/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[0]/2.0 + basegfx::B2DVector(0.5,0.5) , Radius*unScaledTexCoords[0]/2.0 + basegfx::B2DVector(0.5,0.5) );

        basegfx::B3DVector axis(randNormVectorInXYPlane());
        EnteringSlide.Operations.push_back( new SRotate( axis , basegfx::B3DVector(0,0,0) , 180, true, Radius/2.0 , (NextRadius + 1)/2.0 ) );
        LeavingSlide.Operations.push_back( new SRotate( axis , basegfx::B3DVector(0,0,0) , 180, true, Radius/2.0 , (NextRadius + 1)/2.0 ) );
        EnteringSlide.Operations.push_back( new SRotate( axis , basegfx::B3DVector(0,0,0) , -180, false,0.0,1.0) );

        maEnteringSlidePrimitives.push_back(EnteringSlide);
        maLeavingSlidePrimitives.push_back(LeavingSlide);

        LastRadius = Radius;
        Radius = NextRadius;
        NextRadius += dRadius;
    }
    {
        Radius = sqrt(2.0);
        Primitive LeavingSlide;
        Primitive EnteringSlide;
        for(int Side(0); Side < nPointsOnCircles - 1; ++Side)
        {

            EnteringSlide.pushTriangle(clamp(Radius*unScaledTexCoords[Side])/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[Side]/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[Side + 1]/2.0 + basegfx::B2DVector(0.5,0.5) );
            EnteringSlide.pushTriangle(clamp(Radius*unScaledTexCoords[Side])/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[Side + 1]/2.0 + basegfx::B2DVector(0.5,0.5) , clamp(Radius*unScaledTexCoords[Side + 1])/2.0 + basegfx::B2DVector(0.5,0.5) );

            LeavingSlide.pushTriangle(clamp(Radius*unScaledTexCoords[Side])/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[Side]/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[Side + 1]/2.0 + basegfx::B2DVector(0.5,0.5) );
            LeavingSlide.pushTriangle(clamp(Radius*unScaledTexCoords[Side])/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[Side + 1]/2.0 + basegfx::B2DVector(0.5,0.5) , clamp(Radius*unScaledTexCoords[Side + 1])/2.0 + basegfx::B2DVector(0.5,0.5) );
        }

        EnteringSlide.pushTriangle(clamp(Radius*unScaledTexCoords[nPointsOnCircles - 1])/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[nPointsOnCircles - 1]/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[0]/2.0 + basegfx::B2DVector(0.5,0.5) );
        EnteringSlide.pushTriangle(clamp(Radius*unScaledTexCoords[nPointsOnCircles - 1])/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[0]/2.0 + basegfx::B2DVector(0.5,0.5) , clamp(Radius*unScaledTexCoords[0])/2.0 + basegfx::B2DVector(0.5,0.5) );

        LeavingSlide.pushTriangle(clamp(Radius*unScaledTexCoords[nPointsOnCircles - 1])/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[nPointsOnCircles - 1]/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[0]/2.0 + basegfx::B2DVector(0.5,0.5) );
        LeavingSlide.pushTriangle(clamp(Radius*unScaledTexCoords[nPointsOnCircles - 1])/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[0]/2.0 + basegfx::B2DVector(0.5,0.5) , clamp(Radius*unScaledTexCoords[0])/2.0 + basegfx::B2DVector(0.5,0.5) );

        basegfx::B3DVector axis(randNormVectorInXYPlane());
        EnteringSlide.Operations.push_back( new SRotate( axis , basegfx::B3DVector(0,0,0) , 180, true, (LastRadius + dRadius)/2.0 , 1.0 ) );
        LeavingSlide.Operations.push_back( new SRotate( axis , basegfx::B3DVector(0,0,0) , 180, true, (LastRadius + dRadius)/2.0 , 1.0 ) );
        EnteringSlide.Operations.push_back( new SRotate( axis , basegfx::B3DVector(0,0,0) , -180, false,0.0,1.0) );

        maEnteringSlidePrimitives.push_back(EnteringSlide);
        maLeavingSlidePrimitives.push_back(LeavingSlide);
    }
}

void OGLTransitionImpl::makeHelix( ::sal_uInt16 nRows )
{
    clear();
    double invN(1.0/static_cast<double>(nRows));
    double iDn = 0.0;
    double iPDn = invN;
    for(unsigned int i(0); i < nRows; ++i)
    {
        Primitive Tile;

        Tile.pushTriangle(basegfx::B2DVector( 1.0 , iDn ) , basegfx::B2DVector( 0.0 , iDn ) , basegfx::B2DVector( 0.0 , iPDn ));

        Tile.pushTriangle(basegfx::B2DVector( 1.0 , iPDn ) , basegfx::B2DVector( 1.0 , iDn ) , basegfx::B2DVector( 0.0 , iPDn ));

        Tile.Operations.push_back( new SRotate( basegfx::B3DVector( 0 , 1 , 0 ) , ( Tile.getVertices()[1] + Tile.getVertices()[3] )/2.0 , 180 ,
                                                true,min(max(static_cast<double>(i - nRows/2.0)*invN/2.0,0.0),1.0),
                                                min(max(static_cast<double>(i + nRows/2.0)*invN/2.0,0.0),1.0) ) );

        maLeavingSlidePrimitives.push_back(Tile);

        Tile.Operations.push_back( new SRotate( basegfx::B3DVector( 0 , 1 , 0 ) , ( Tile.getVertices()[1] + Tile.getVertices()[3] )/2.0 , -180 , false,0.0,1.0) );

        maEnteringSlidePrimitives.push_back(Tile);

        iDn += invN;
        iPDn += invN;
    }
}

void OGLTransitionImpl::makeNByMTileFlip( ::sal_uInt16 n, ::sal_uInt16 m )
{
    clear();
    double invN(1.0/static_cast<double>(n));
    double invM(1.0/static_cast<double>(m));
    double iDn = 0.0;
    double iPDn = invN;
    for(unsigned int i(0); i < n; ++i)
    {
        double jDm = 0.0;
        double jPDm = invM;
        for(unsigned int j(0); j < m; ++j)
        {
            Primitive Tile;

            Tile.pushTriangle(basegfx::B2DVector( iPDn , jDm ) , basegfx::B2DVector( iDn , jDm ) , basegfx::B2DVector( iDn , jPDm ));

            Tile.pushTriangle(basegfx::B2DVector( iPDn , jPDm ) , basegfx::B2DVector( iPDn , jDm ) , basegfx::B2DVector( iDn , jPDm ));//bottom left corner of tile

            Tile.Operations.push_back( new SRotate( basegfx::B3DVector( 1 , 1 , 0 ) , ( Tile.getVertices()[1] + Tile.getVertices()[3] )/2.0 , 180 , true, iDn*jDm/2.0 , ((iPDn*jPDm)+1.0)/2.0 ) );
            maLeavingSlidePrimitives.push_back(Tile);
            Tile.Operations.push_back( new SRotate( basegfx::B3DVector( 1 , 1 , 0 ) , ( Tile.getVertices()[1] + Tile.getVertices()[3] )/2.0 , -180, false, iDn*jDm/2.0 , ((iPDn*jPDm)+1.0)/2.0 ) );

            maEnteringSlidePrimitives.push_back(Tile);

            jDm += invM;
            jPDm += invM;
        }
        iDn += invN;
        iPDn += invN;
    }
}

SRotate::SRotate(const basegfx::B3DVector& Axis,const basegfx::B3DVector& Origin,double Angle, bool bInter, double T0, double T1):axis(Axis),origin(Origin),angle(Angle)
{
    nT0 = T0;
    nT1 = T1;
    bInterpolate = bInter;
}

SScale::SScale(const basegfx::B3DVector& Scale,const basegfx::B3DVector& Origin, bool bInter, double T0, double T1):scale(Scale),origin(Origin)
{
    nT0 = T0;
    nT1 = T1;
    bInterpolate = bInter;
}

RotateAndScaleDepthByWidth::RotateAndScaleDepthByWidth(const basegfx::B3DVector& Axis,const basegfx::B3DVector& Origin,double Angle, bool bInter, double T0, double T1):axis(Axis),origin(Origin),angle(Angle)
{
    nT0 = T0;
    nT1 = T1;
    bInterpolate = bInter;
}

RotateAndScaleDepthByHeight::RotateAndScaleDepthByHeight(const basegfx::B3DVector& Axis,const basegfx::B3DVector& Origin,double Angle, bool bInter, double T0, double T1):axis(Axis),origin(Origin),angle(Angle)
{
    nT0 = T0;
    nT1 = T1;
    bInterpolate = bInter;
}


STranslate::STranslate(const basegfx::B3DVector& Vector, bool bInter, double T0, double T1):vector(Vector)
{
    nT0 = T0;
    nT1 = T1;
    bInterpolate = bInter;
}

inline double intervalInter(double t, double T0, double T1)
{
    return ( t - T0 ) / ( T1 - T0 );
}

void STranslate::interpolate(double t,double SlideWidthScale,double SlideHeightScale)
{
    if(t <= nT0)
        return;
    if(!bInterpolate || t > nT1)
        t = nT1;
    t = intervalInter(t,nT0,nT1);
    glTranslated(SlideWidthScale*t*vector.getX(),SlideHeightScale*t*vector.getY(),t*vector.getZ());
}

void SRotate::interpolate(double t,double SlideWidthScale,double SlideHeightScale)
{
    if(t <= nT0)
        return;
    if(!bInterpolate || t > nT1)
        t = nT1;
    t = intervalInter(t,nT0,nT1);
    glTranslated(SlideWidthScale*origin.getX(),SlideHeightScale*origin.getY(),origin.getZ());
    glScaled(SlideWidthScale,SlideHeightScale,1);
    glRotated(t*angle,axis.getX(),axis.getY(),axis.getZ());
    glScaled(1/SlideWidthScale,1/SlideHeightScale,1);
    glTranslated(-SlideWidthScale*origin.getX(),-SlideHeightScale*origin.getY(),-origin.getZ());
}

void SScale::interpolate(double t,double SlideWidthScale,double SlideHeightScale)
{
    if(t <= nT0)
        return;
    if(!bInterpolate || t > nT1)
        t = nT1;
    t = intervalInter(t,nT0,nT1);
    glTranslated(SlideWidthScale*origin.getX(),SlideHeightScale*origin.getY(),origin.getZ());
    glScaled((1-t) + t*scale.getX(),(1-t) + t*scale.getY(),(1-t) + t*scale.getZ());
    glTranslated(-SlideWidthScale*origin.getX(),-SlideHeightScale*origin.getY(),-origin.getZ());
}

void RotateAndScaleDepthByWidth::interpolate(double t,double SlideWidthScale,double SlideHeightScale)
{
    if(t <= nT0)
        return;
    if(!bInterpolate || t > nT1)
        t = nT1;
    t = intervalInter(t,nT0,nT1);
    glTranslated(SlideWidthScale*origin.getX(),SlideHeightScale*origin.getY(),SlideWidthScale*origin.getZ());
    glRotated(t*angle,axis.getX(),axis.getY(),axis.getZ());
    glTranslated(-SlideWidthScale*origin.getX(),-SlideHeightScale*origin.getY(),-SlideWidthScale*origin.getZ());
}

void RotateAndScaleDepthByHeight::interpolate(double t,double SlideWidthScale,double SlideHeightScale)
{
    if(t <= nT0)
        return;
    if(!bInterpolate || t > nT1)
        t = nT1;
    t = intervalInter(t,nT0,nT1);
    glTranslated(SlideWidthScale*origin.getX(),SlideHeightScale*origin.getY(),SlideHeightScale*origin.getZ());
    glRotated(t*angle,axis.getX(),axis.getY(),axis.getZ());
    glTranslated(-SlideWidthScale*origin.getX(),-SlideHeightScale*origin.getY(),-SlideHeightScale*origin.getZ());
}

SEllipseTranslate::SEllipseTranslate(double dWidth, double dHeight, double dStartPosition, double dEndPosition, bool bInter, double T0, double T1)
{
    nT0 = T0;
    nT1 = T1;
    bInterpolate = bInter;
    width = dWidth;
    height = dHeight;
    startPosition = dStartPosition;
    endPosition = dEndPosition;
}

void SEllipseTranslate::interpolate(double t,double /* SlideWidthScale */,double /* SlideHeightScale */)
{
    if(t <= nT0)
        return;
    if(!bInterpolate || t > nT1)
        t = nT1;
    t = intervalInter(t,nT0,nT1);

    double a1, a2, x, y;
    a1 = startPosition*2*M_PI;
    a2 = (startPosition + t*(endPosition - startPosition))*2*M_PI;
    x = width*(cos (a2) - cos (a1))/2;
    y = height*(sin (a2) - sin (a1))/2;

    glTranslated(x, 0, y);
}

STranslate* STranslate::clone()
{
    return new STranslate(*this);
}
SRotate* SRotate::clone()
{
    return new SRotate(*this);
}

SScale* SScale::clone()
{
    return new SScale(*this);
}

SEllipseTranslate* SEllipseTranslate::clone()
{
    return new SEllipseTranslate(*this);
}

RotateAndScaleDepthByWidth* RotateAndScaleDepthByWidth::clone()
{
    return new RotateAndScaleDepthByWidth(*this);
}

RotateAndScaleDepthByHeight* RotateAndScaleDepthByHeight::clone()
{
    return new RotateAndScaleDepthByHeight(*this);
}

Primitive& Primitive::operator=(const Primitive& rvalue)
{
    for(unsigned int i( 0 ); i < rvalue.Operations.size(); ++i)
        Operations.push_back(rvalue.Operations[i]->clone());
    for(unsigned int i( 0 ); i < rvalue.Vertices.size(); ++i)//SPEED! use copy or something. this is slow.
        Vertices.push_back(rvalue.Vertices[i]);
    for(unsigned int i( 0 ); i < rvalue.TexCoords.size(); ++i)//SPEED! use copy or something. this is slow.
        TexCoords.push_back(rvalue.TexCoords[i]);
    for(unsigned int i( 0 ); i < rvalue.Normals.size(); ++i)//SPEED! use copy or something. this is slow.
        Normals.push_back(rvalue.Normals[i]);
    return *this;
}

Primitive::Primitive(const Primitive& rvalue)
{
    for(unsigned int i( 0 ); i < rvalue.Operations.size(); ++i)
        Operations.push_back(rvalue.Operations[i]->clone());
    for(unsigned int i( 0 ); i < rvalue.Vertices.size(); ++i)//SPEED! use copy or something. this is slow.
        Vertices.push_back(rvalue.Vertices[i]);
    for(unsigned int i( 0 ); i < rvalue.TexCoords.size(); ++i)//SPEED! use copy or something. this is slow.
        TexCoords.push_back(rvalue.TexCoords[i]);
    for(unsigned int i( 0 ); i < rvalue.Normals.size(); ++i)//SPEED! use copy or something. this is slow.
        Normals.push_back(rvalue.Normals[i]);
}

void Primitive::pushTriangle(const basegfx::B2DVector& SlideLocation0,const basegfx::B2DVector& SlideLocation1,const basegfx::B2DVector& SlideLocation2)
{
    vector<basegfx::B3DVector> Verts;
    vector<basegfx::B2DVector> Texs;
    Verts.reserve(3);
    Texs.reserve(3);

    Verts.push_back(basegfx::B3DVector( 2*SlideLocation0.getX() - 1, -2*SlideLocation0.getY() + 1 , 0.0 ));
    Verts.push_back(basegfx::B3DVector( 2*SlideLocation1.getX() - 1, -2*SlideLocation1.getY() + 1 , 0.0 ));
    Verts.push_back(basegfx::B3DVector( 2*SlideLocation2.getX() - 1, -2*SlideLocation2.getY() + 1 , 0.0 ));

    //figure out if they're facing the correct way, and make them face the correct way.
    basegfx::B3DVector Normal( basegfx::cross( Verts[0] - Verts[1] , Verts[1] - Verts[2] ) );
    if(Normal.getZ() >= 0.0)//if the normal is facing us
    {
        Texs.push_back(SlideLocation0);
        Texs.push_back(SlideLocation1);
        Texs.push_back(SlideLocation2);
    }
    else // if the normal is facing away from us, make it face us
    {
        Texs.push_back(SlideLocation0);
        Texs.push_back(SlideLocation2);
        Texs.push_back(SlideLocation1);
        Verts.clear();
        Verts.push_back(basegfx::B3DVector( 2*SlideLocation0.getX() - 1, -2*SlideLocation0.getY() + 1 , 0.0 ));
        Verts.push_back(basegfx::B3DVector( 2*SlideLocation2.getX() - 1, -2*SlideLocation2.getY() + 1 , 0.0 ));
        Verts.push_back(basegfx::B3DVector( 2*SlideLocation1.getX() - 1, -2*SlideLocation1.getY() + 1 , 0.0 ));
    }

    Vertices.push_back(Verts[0]);
    Vertices.push_back(Verts[1]);
    Vertices.push_back(Verts[2]);

    TexCoords.push_back(Texs[0]);
    TexCoords.push_back(Texs[1]);
    TexCoords.push_back(Texs[2]);

    Normals.push_back(basegfx::B3DVector(0,0,1));//all normals always face the screen when untransformed.
    Normals.push_back(basegfx::B3DVector(0,0,1));//all normals always face the screen when untransformed.
    Normals.push_back(basegfx::B3DVector(0,0,1));//all normals always face the screen when untransformed.
}

void OGLTransitionImpl::makeDiamond()
{
    mmPrepare = &OGLTransitionImpl::prepareDiamond;
    mbUseMipMapLeaving = mbUseMipMapEntering = false;
}

void OGLTransitionImpl::prepareDiamond( double nTime, double /* SlideWidth */, double /* SlideHeight */, double /* DispWidth */, double /* DispHeight */ )
{
    Primitive Slide1, Slide2;

    Slide1.pushTriangle (basegfx::B2DVector (0,0), basegfx::B2DVector (1,0), basegfx::B2DVector (0,1));
    Slide1.pushTriangle (basegfx::B2DVector (1,0), basegfx::B2DVector (0,1), basegfx::B2DVector (1,1));
    maEnteringSlidePrimitives.push_back (Slide1);


    if( nTime >= 0.5 ) {
        double m = 1 - nTime;

        Slide2.pushTriangle (basegfx::B2DVector (0,0), basegfx::B2DVector (m,0), basegfx::B2DVector (0,m));
        Slide2.pushTriangle (basegfx::B2DVector (nTime,0), basegfx::B2DVector (1,0), basegfx::B2DVector (1,m));
        Slide2.pushTriangle (basegfx::B2DVector (1,nTime), basegfx::B2DVector (1,1), basegfx::B2DVector (nTime,1));
        Slide2.pushTriangle (basegfx::B2DVector (0,nTime), basegfx::B2DVector (m,1), basegfx::B2DVector (0,1));
    } else {
        double l = 0.5 - nTime;
        double h = 0.5 + nTime;

        Slide2.pushTriangle (basegfx::B2DVector (0,0), basegfx::B2DVector (1,0), basegfx::B2DVector (0.5,l));
        Slide2.pushTriangle (basegfx::B2DVector (0.5,l), basegfx::B2DVector (1,0), basegfx::B2DVector (h,0.5));
        Slide2.pushTriangle (basegfx::B2DVector (1,0), basegfx::B2DVector (1,1), basegfx::B2DVector (h,0.5));
        Slide2.pushTriangle (basegfx::B2DVector (h,0.5), basegfx::B2DVector (1,1), basegfx::B2DVector (0.5,h));
        Slide2.pushTriangle (basegfx::B2DVector (0.5,h), basegfx::B2DVector (1,1), basegfx::B2DVector (0,1));
        Slide2.pushTriangle (basegfx::B2DVector (l,0.5), basegfx::B2DVector (0.5,h), basegfx::B2DVector (0,1));
        Slide2.pushTriangle (basegfx::B2DVector (0,0), basegfx::B2DVector (l,0.5), basegfx::B2DVector (0,1));
        Slide2.pushTriangle (basegfx::B2DVector (0,0), basegfx::B2DVector (0.5,l), basegfx::B2DVector (l,0.5));
    }
    Slide2.Operations.push_back (new STranslate (basegfx::B3DVector (0, 0, 0.00000001), false, -1, 0));
    maLeavingSlidePrimitives.push_back (Slide2);
}

void OGLTransitionImpl::makeVenetianBlinds( bool vertical, int parts )
{
    static double t30 = tan( M_PI/6.0 );
    double n, ln = 0;
    double p = 1.0/parts;

    for( int i=0; i<parts; i++ ) {
        Primitive Slide;
        n = (i + 1)/(double)parts;
        if( vertical ) {
            Slide.pushTriangle (basegfx::B2DVector (ln,0), basegfx::B2DVector (n,0), basegfx::B2DVector (ln,1));
            Slide.pushTriangle (basegfx::B2DVector (n,0), basegfx::B2DVector (ln,1), basegfx::B2DVector (n,1));
            Slide.Operations.push_back(new RotateAndScaleDepthByWidth(basegfx::B3DVector(0, 1, 0), basegfx::B3DVector(n + ln - 1, 0, -t30*p), -120, true, 0.0, 1.0));
        } else {
            Slide.pushTriangle (basegfx::B2DVector (0,ln), basegfx::B2DVector (1,ln), basegfx::B2DVector (0,n));
            Slide.pushTriangle (basegfx::B2DVector (1,ln), basegfx::B2DVector (0,n), basegfx::B2DVector (1,n));
            Slide.Operations.push_back(new RotateAndScaleDepthByHeight(basegfx::B3DVector(1, 0, 0), basegfx::B3DVector(0, 1 - n - ln, -t30*p), -120, true, 0.0, 1.0));
        }
        maLeavingSlidePrimitives.push_back (Slide);

        if( vertical ) {
            Slide.Operations.push_back(new SRotate(basegfx::B3DVector(0, 1, 0), basegfx::B3DVector(2*n - 1, 0, 0), -60, false, -1, 0));
            Slide.Operations.push_back(new SRotate(basegfx::B3DVector(0, 1, 0), basegfx::B3DVector(n + ln - 1, 0, 0), 180, false, -1, 0));
        } else {
            Slide.Operations.push_back(new SRotate(basegfx::B3DVector(1, 0, 0), basegfx::B3DVector(0, 1 - 2*n, 0), -60, false, -1, 0));
            Slide.Operations.push_back(new SRotate(basegfx::B3DVector(1, 0, 0), basegfx::B3DVector(0, 1 - n - ln, 0), 180, false, -1, 0));
        }
        maEnteringSlidePrimitives.push_back (Slide);
        ln = n;
    }
}

void OGLTransitionImpl::displaySlidesFadeSmoothly( double nTime, ::sal_Int32 glLeavingSlideTex, ::sal_Int32 glEnteringSlideTex, double SlideWidthScale, double SlideHeightScale )
{
    applyOverallOperations( nTime, SlideWidthScale, SlideHeightScale );

    glDisable(GL_DEPTH_TEST);

    displaySlide( nTime, glLeavingSlideTex, maLeavingSlidePrimitives, SlideWidthScale, SlideHeightScale );

    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glColor4f( 1, 1, 1, nTime );
    displaySlide( nTime, glEnteringSlideTex, maEnteringSlidePrimitives, SlideWidthScale, SlideHeightScale );
    glDisable(GL_BLEND);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glEnable(GL_LIGHTING);

    glEnable(GL_DEPTH_TEST);
}

void OGLTransitionImpl::makeFadeSmoothly()
{
    Primitive Slide;

    Slide.pushTriangle (basegfx::B2DVector (0,0), basegfx::B2DVector (1,0), basegfx::B2DVector (0,1));
    Slide.pushTriangle (basegfx::B2DVector (1,0), basegfx::B2DVector (0,1), basegfx::B2DVector (1,1));
    maLeavingSlidePrimitives.push_back (Slide);
    maEnteringSlidePrimitives.push_back (Slide);

    mmDisplaySlides = &OGLTransitionImpl::displaySlidesFadeSmoothly;
    mbUseMipMapLeaving = mbUseMipMapEntering = false;
}

void OGLTransitionImpl::displaySlidesFadeThroughBlack( double nTime, ::sal_Int32 glLeavingSlideTex, ::sal_Int32 glEnteringSlideTex, double SlideWidthScale, double SlideHeightScale )
{
    applyOverallOperations( nTime, SlideWidthScale, SlideHeightScale );

    glDisable(GL_DEPTH_TEST);

    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    if( nTime < 0.5 ) {
    glColor4f( 1, 1, 1, 1 - nTime*2 );
    displaySlide( nTime, glLeavingSlideTex, maLeavingSlidePrimitives, SlideWidthScale, SlideHeightScale );
    } else {
    glColor4f( 1, 1, 1, (nTime - 0.5)*2 );
    displaySlide( nTime, glEnteringSlideTex, maEnteringSlidePrimitives, SlideWidthScale, SlideHeightScale );
    }
    glDisable(GL_BLEND);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glEnable(GL_LIGHTING);

    glEnable(GL_DEPTH_TEST);
}

void OGLTransitionImpl::makeFadeThroughBlack()
{
    Primitive Slide;

    Slide.pushTriangle (basegfx::B2DVector (0,0), basegfx::B2DVector (1,0), basegfx::B2DVector (0,1));
    Slide.pushTriangle (basegfx::B2DVector (1,0), basegfx::B2DVector (0,1), basegfx::B2DVector (1,1));
    maLeavingSlidePrimitives.push_back (Slide);
    maEnteringSlidePrimitives.push_back (Slide);

    mmDisplaySlides = &OGLTransitionImpl::displaySlidesFadeThroughBlack;
    mbUseMipMapLeaving = mbUseMipMapEntering = false;
}

static const char* basicVertexShader = "\n\
varying vec2 v_texturePosition;\n\
\n\
void main( void )\n\
{\n\
    gl_Position = ftransform();\n\
    v_texturePosition = gl_MultiTexCoord0.xy;\n\
}\n\
";

static const char* staticFragmentShader = "\n\
uniform sampler2D leavingSlideTexture;\n\
uniform sampler2D enteringSlideTexture;\n\
uniform sampler2D permTexture;\n\
uniform float time;\n\
varying vec2 v_texturePosition;\n\
\n\
float snoise(vec2 P) {\n\
\n\
  return texture2D(permTexture, P).r;\n\
}\n\
\n\
\n\
#define PART 0.5\n\
#define START 0.4\n\
#define END 0.9\n\
\n\
void main() {\n\
    float sn = snoise(10.0*v_texturePosition+time*0.07);\n\
    if( time < PART ) {\n\
        float sn1 = snoise(vec2(time*15.0, 20.0*v_texturePosition.y));\n\
        float sn2 = snoise(v_texturePosition);\n\
        if (sn1 > 1.0 - time*time && sn2 < 2.0*time+0.1)\n\
            gl_FragColor = vec4(sn, sn, sn, 1.0);\n\
        else if (time > START )\n\
            gl_FragColor = ((time-START)/(PART - START))*vec4(sn, sn, sn, 1.0) + (1.0 - (time - START)/(PART - START))*texture2D(leavingSlideTexture, v_texturePosition);\n\
        else\n\
            gl_FragColor = texture2D(leavingSlideTexture, v_texturePosition);\n\
    } else if ( time < PART ) {\n\
            gl_FragColor = texture2D(leavingSlideTexture, v_texturePosition);\n\
    } else if ( time > END ) {\n\
        gl_FragColor = ((1.0 - time)/(1.0 - END))*vec4(sn, sn, sn, 1.0) + ((time - END)/(1.0 - END))*texture2D(enteringSlideTexture, v_texturePosition);\n\
    } else \n\
        gl_FragColor = vec4(sn, sn, sn, 1.0);\n\
}\n\
";

static const char* dissolveFragmentShader = "\n\
uniform sampler2D leavingSlideTexture;\n\
uniform sampler2D enteringSlideTexture;\n\
uniform sampler2D permTexture;\n\
uniform float time;\n\
varying vec2 v_texturePosition;\n\
\n\
float snoise(vec2 P) {\n\
\n\
  return texture2D(permTexture, P).r;\n\
}\n\
\n\
void main() {\n\
     float sn = snoise(10.0*v_texturePosition);\n\
     if( sn < time)\n\
         gl_FragColor = texture2D(enteringSlideTexture, v_texturePosition);\n\
     else\n\
         gl_FragColor = texture2D(leavingSlideTexture, v_texturePosition);\n\
}\n\
";

int permutation256 [256]= {
215, 100, 200, 204, 233,  50,  85, 196,
 71, 141, 122, 160,  93, 131, 243, 234,
162, 183,  36, 155,   4,  62,  35, 205,
 40, 102,  33,  27, 255,  55, 214, 156,
 75, 163, 134, 126, 249,  74, 197, 228,
 72,  90, 206, 235,  17,  22,  49, 169,
227,  89,  16,   5, 117,  60, 248, 230,
217,  68, 138,  96, 194, 170, 136,  10,
112, 238, 184, 189, 176,  42, 225, 212,
 84,  58, 175, 244, 150, 168, 219, 236,
101, 208, 123,  37, 164, 110, 158, 201,
 78, 114,  57,  48,  70, 142, 106,  43,
232,  26,  32, 252, 239,  98, 191,  94,
 59, 149,  39, 187, 203, 190,  19,  13,
133,  45,  61, 247,  23,  34,  20,  52,
118, 209, 146, 193, 222,  18,   1, 152,
 46,  41,  91, 148, 115,  25, 135,  77,
254, 147, 224, 161,   9, 213, 223, 250,
231, 251, 127, 166,  63, 179,  81, 130,
139,  28, 120, 151, 241,  86, 111,   0,
 88, 153, 172, 182, 159, 105, 178,  47,
 51, 167,  65,  66,  92,  73, 198, 211,
245, 195,  31, 220, 140,  76, 221, 186,
154, 185,  56,  83,  38, 165, 109,  67,
124, 226, 132,  53, 229,  29,  12, 181,
121,  24, 207, 199, 177, 113,  30,  80,
  3,  97, 188,  79, 216, 173,   8, 145,
 87, 128, 180, 237, 240, 137, 125, 104,
 15, 242, 119, 246, 103, 143,  95, 144,
  2,  44,  69, 157, 192, 174,  14,  54,
218,  82,  64, 210,  11,   6, 129,  21,
116, 171,  99, 202,   7, 107, 253, 108
};

void initPermTexture(GLuint *texID)
{
  glGenTextures(1, texID);
  glBindTexture(GL_TEXTURE_2D, *texID);

  static bool initialized = false;
  static unsigned char permutation2D[256*256*4];
  if( !initialized ) {
      int x, y;

      for( y=0; y < 256; y++ )
          for( x=0; x < 256; x++ )
              permutation2D[x*4 + y*1024] = permutation256[(y + permutation256[x]) & 0xff];

      initialized = true;
  }

  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, permutation2D );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
}

void OGLTransitionImpl::preparePermShader()
{
#ifdef GL_VERSION_2_0
    if( mProgramObject ) {
        OGLShaders::glUseProgram( mProgramObject );

        GLint location = OGLShaders::glGetUniformLocation( mProgramObject, "leavingSlideTexture" );
        if( location != -1 ) {
            OGLShaders::glUniform1i( location, 0 );  // texture unit 0
        }

        glActiveTexture(GL_TEXTURE1);
        if( !maHelperTexture )
            initPermTexture( &maHelperTexture );
        glActiveTexture(GL_TEXTURE0);

        location = OGLShaders::glGetUniformLocation( mProgramObject, "permTexture" );
        if( location != -1 ) {
            OGLShaders::glUniform1i( location, 1 );  // texture unit 1
        }

        location = OGLShaders::glGetUniformLocation( mProgramObject, "enteringSlideTexture" );
        if( location != -1 ) {
            OGLShaders::glUniform1i( location, 2 );  // texture unit 2
        }
    }
#endif
}

void OGLTransitionImpl::prepareStatic( ::sal_Int32 /* glLeavingSlideTex */, ::sal_Int32 /* glEnteringSlideTex */ )
{
    mProgramObject = OGLShaders::LinkProgram( basicVertexShader, staticFragmentShader );

    preparePermShader();
}

void OGLTransitionImpl::displaySlidesShaders( double nTime, ::sal_Int32 glLeavingSlideTex, ::sal_Int32 glEnteringSlideTex,
                                              double SlideWidthScale, double SlideHeightScale )
{
    applyOverallOperations( nTime, SlideWidthScale, SlideHeightScale );

#ifdef GL_VERSION_2_0
    if( mProgramObject ) {
        GLint location = OGLShaders::glGetUniformLocation( mProgramObject, "time" );
        if( location != -1 ) {
            OGLShaders::glUniform1f( location, nTime );
        }
    }

    glActiveTexture( GL_TEXTURE2 );
    glBindTexture( GL_TEXTURE_2D, glEnteringSlideTex );
    glActiveTexture( GL_TEXTURE0 );
#endif

    displaySlide( nTime, glLeavingSlideTex, maLeavingSlidePrimitives, SlideWidthScale, SlideHeightScale );
}

void OGLTransitionImpl::makeStatic()
{
    Primitive Slide;

    Slide.pushTriangle (basegfx::B2DVector (0,0), basegfx::B2DVector (1,0), basegfx::B2DVector (0,1));
    Slide.pushTriangle (basegfx::B2DVector (1,0), basegfx::B2DVector (0,1), basegfx::B2DVector (1,1));
    maLeavingSlidePrimitives.push_back (Slide);
    maEnteringSlidePrimitives.push_back (Slide);

    mmDisplaySlides = &OGLTransitionImpl::displaySlidesShaders;
    mmPrepareTransition = &OGLTransitionImpl::prepareStatic;
    mbUseMipMapLeaving = mbUseMipMapEntering = false;

    mnRequiredGLVersion = 2.0;
}

void OGLTransitionImpl::prepareDissolve( ::sal_Int32 /* glLeavingSlideTex */, ::sal_Int32 /* glEnteringSlideTex */ )
{
    mProgramObject = OGLShaders::LinkProgram( basicVertexShader, dissolveFragmentShader );

    preparePermShader();
}

void OGLTransitionImpl::makeDissolve()
{
    Primitive Slide;

    Slide.pushTriangle (basegfx::B2DVector (0,0), basegfx::B2DVector (1,0), basegfx::B2DVector (0,1));
    Slide.pushTriangle (basegfx::B2DVector (1,0), basegfx::B2DVector (0,1), basegfx::B2DVector (1,1));
    maLeavingSlidePrimitives.push_back (Slide);
    maEnteringSlidePrimitives.push_back (Slide);

    mmDisplaySlides = &OGLTransitionImpl::displaySlidesShaders;
    mmPrepareTransition = &OGLTransitionImpl::prepareDissolve;
    mbUseMipMapLeaving = mbUseMipMapEntering = false;

    mnRequiredGLVersion = 2.0;
}

void OGLTransitionImpl::makeNewsflash()
{
    Primitive Slide;

    Slide.pushTriangle(basegfx::B2DVector(0,0),basegfx::B2DVector(1,0),basegfx::B2DVector(0,1));
    Slide.pushTriangle(basegfx::B2DVector(1,0),basegfx::B2DVector(0,1),basegfx::B2DVector(1,1));
    Slide.Operations.push_back(new SRotate(basegfx::B3DVector(0,0,1),basegfx::B3DVector(0,0,0),3000,true,0,0.5));
    Slide.Operations.push_back(new SScale(basegfx::B3DVector(0.01,0.01,0.01),basegfx::B3DVector(0,0,0),true,0,0.5));
    Slide.Operations.push_back(new STranslate(basegfx::B3DVector(-10000, 0, 0),false, 0.5, 2));
    maLeavingSlidePrimitives.push_back(Slide);

    Slide.Operations.clear();
    Slide.Operations.push_back(new SRotate(basegfx::B3DVector(0,0,1),basegfx::B3DVector(0,0,0),-3000,true,0.5,1));
    Slide.Operations.push_back(new STranslate(basegfx::B3DVector(-100, 0, 0),false, -1, 1));
    Slide.Operations.push_back(new STranslate(basegfx::B3DVector(100, 0, 0),false, 0.5, 1));
    Slide.Operations.push_back(new SScale(basegfx::B3DVector(0.01,0.01,0.01),basegfx::B3DVector(0,0,0),false,-1,1));
    Slide.Operations.push_back(new SScale(basegfx::B3DVector(100,100,100),basegfx::B3DVector(0,0,0),true,0.5,1));
    maEnteringSlidePrimitives.push_back(Slide);

    OverallOperations.push_back(new SRotate(basegfx::B3DVector(0,0,1),basegfx::B3DVector(0.2,0.2,0),1080,true,0,1));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
