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

#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vcl/opengl/OpenGLHelper.hxx>

#include <algorithm>
#include <utility>

#include <comphelper/random.hxx>

#include "OGLTrans_TransitionImpl.hxx"
#include <math.h>

TransitionScene::TransitionScene(TransitionScene const& rOther)
    : maLeavingSlidePrimitives(rOther.maLeavingSlidePrimitives)
    , maEnteringSlidePrimitives(rOther.maEnteringSlidePrimitives)
    , maOverallOperations(rOther.maOverallOperations)
    , maSceneObjects(rOther.maSceneObjects)
{
}

TransitionScene& TransitionScene::operator=(const TransitionScene& rOther)
{
    TransitionScene aTmp(rOther);
    swap(aTmp);
    return *this;
}

void TransitionScene::swap(TransitionScene& rOther)
{
    using std::swap;

    swap(maLeavingSlidePrimitives, rOther.maLeavingSlidePrimitives);
    swap(maEnteringSlidePrimitives, rOther.maEnteringSlidePrimitives);
    swap(maOverallOperations, rOther.maOverallOperations);
    swap(maSceneObjects, rOther.maSceneObjects);
}

OGLTransitionImpl::~OGLTransitionImpl()
{
}

void OGLTransitionImpl::setScene(TransitionScene const& rScene)
{
    maScene = rScene;
}

void OGLTransitionImpl::prepare( sal_Int32 glLeavingSlideTex, sal_Int32 glEnteringSlideTex )
{
    const SceneObjects_t& rSceneObjects(maScene.getSceneObjects());
    for(size_t i(0); i != rSceneObjects.size(); ++i) {
        rSceneObjects[i]->prepare();
    }

    prepareTransition( glLeavingSlideTex, glEnteringSlideTex );
}

void OGLTransitionImpl::finish()
{
    const SceneObjects_t& rSceneObjects(maScene.getSceneObjects());
    for(size_t i(0); i != rSceneObjects.size(); ++i) {
        rSceneObjects[i]->finish();
    }

    finishTransition();
}

static void blendSlide( double depth )
{
    CHECK_GL_ERROR();
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
    CHECK_GL_ERROR();
}

static void slideShadow( double nTime, const Primitive& primitive, double sw, double sh )
{
    CHECK_GL_ERROR();
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
    CHECK_GL_ERROR();
}

void OGLTransitionImpl::prepare( double, double, double, double, double )
{
}

void OGLTransitionImpl::finish( double, double, double, double, double )
{
}

void OGLTransitionImpl::prepareTransition( sal_Int32, sal_Int32 )
{
}

void OGLTransitionImpl::finishTransition()
{
}

void OGLTransitionImpl::displaySlides_( double nTime, sal_Int32 glLeavingSlideTex, sal_Int32 glEnteringSlideTex, double SlideWidthScale, double SlideHeightScale )
{
    CHECK_GL_ERROR();
    applyOverallOperations( nTime, SlideWidthScale, SlideHeightScale );

    glEnable(GL_TEXTURE_2D);
    displaySlide( nTime, glLeavingSlideTex, maScene.getLeavingSlide(), SlideWidthScale, SlideHeightScale );
    displaySlide( nTime, glEnteringSlideTex, maScene.getEnteringSlide(), SlideWidthScale, SlideHeightScale );
}

void OGLTransitionImpl::display( double nTime, sal_Int32 glLeavingSlideTex, sal_Int32 glEnteringSlideTex,
                                 double SlideWidth, double SlideHeight, double DispWidth, double DispHeight )
{
    const double SlideWidthScale = SlideWidth/DispWidth;
    const double SlideHeightScale = SlideHeight/DispHeight;

    CHECK_GL_ERROR();
    prepare( nTime, SlideWidth, SlideHeight, DispWidth, DispHeight );

    CHECK_GL_ERROR();
    glPushMatrix();
    CHECK_GL_ERROR();
    displaySlides_( nTime, glLeavingSlideTex, glEnteringSlideTex, SlideWidthScale, SlideHeightScale );
    CHECK_GL_ERROR();
    displayScene( nTime, SlideWidth, SlideHeight, DispWidth, DispHeight );
    CHECK_GL_ERROR();
    glPopMatrix();
    CHECK_GL_ERROR();
}

void OGLTransitionImpl::applyOverallOperations( double nTime, double SlideWidthScale, double SlideHeightScale )
{
    const Operations_t& rOverallOperations(maScene.getOperations());
    glm::mat4 matrix;
    for(size_t i(0); i != rOverallOperations.size(); ++i)
        rOverallOperations[i]->interpolate(matrix, nTime, SlideWidthScale, SlideHeightScale);
    CHECK_GL_ERROR();
    glMultMatrixf(glm::value_ptr(matrix));
    CHECK_GL_ERROR();
}

static void display_primitives(const Primitives_t& primitives, double nTime, double WidthScale, double HeightScale)
{
    CHECK_GL_ERROR();
    GLuint buffer;
    glGenBuffers(1, &buffer);
    CHECK_GL_ERROR();
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    int size = 0;
    for (const Primitive& primitive: primitives)
        size += primitive.getVerticesSize();

    CHECK_GL_ERROR();
    glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STREAM_DRAW);
    CHECK_GL_ERROR();
    Vertex *buf = reinterpret_cast<Vertex*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));

    std::vector<int> first_elements;
    int last_pos = 0;
    for (const Primitive& primitive: primitives) {
        first_elements.push_back(last_pos);
        int num = primitive.writeVertices(buf);
        buf += num;
        last_pos += num;
    }
    auto first = first_elements.begin();

    CHECK_GL_ERROR();
    glUnmapBuffer(GL_ARRAY_BUFFER);

    // State initialization
    // TODO: move that elsewhere.
    CHECK_GL_ERROR();
    glEnableClientState( GL_VERTEX_ARRAY );
    CHECK_GL_ERROR();
    glVertexPointer( 3, GL_FLOAT, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)) );
    CHECK_GL_ERROR();
    glEnableClientState( GL_NORMAL_ARRAY );
    CHECK_GL_ERROR();
    glNormalPointer( GL_FLOAT , sizeof(Vertex) , reinterpret_cast<void*>(offsetof(Vertex, normal)) );
    CHECK_GL_ERROR();
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );
    CHECK_GL_ERROR();
    glTexCoordPointer( 2, GL_FLOAT, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, texcoord)) );

    for (const Primitive& primitive: primitives)
        primitive.display(nTime, WidthScale, HeightScale, *first++);

    CHECK_GL_ERROR();
    glDeleteBuffers(1, &buffer);
}

void
OGLTransitionImpl::displaySlide(
        const double nTime,
        const sal_Int32 glSlideTex, const Primitives_t& primitives,
        double SlideWidthScale, double SlideHeightScale )
{
    CHECK_GL_ERROR();
    //TODO change to foreach
    glBindTexture(GL_TEXTURE_2D, glSlideTex);
    CHECK_GL_ERROR();

    // display slide reflection
    // note that depth test is turned off while blending the shadow
    // so the slides has to be rendered in right order, see rochade as example
    if( maSettings.mbReflectSlides ) {
        double surfaceLevel = -0.04;

        /* reflected slides */
        glPushMatrix();

        glm::mat4 matrix;
        matrix = glm::scale(matrix, glm::vec3(1, -1, 1));
        matrix = glm::translate(matrix, glm::vec3(0, 2 - surfaceLevel, 0));
        glMultMatrixf(glm::value_ptr(matrix));

        glCullFace(GL_FRONT);
        display_primitives(primitives, nTime, SlideWidthScale, SlideHeightScale);
        glCullFace(GL_BACK);

        slideShadow( nTime, primitives[0], SlideWidthScale, SlideHeightScale );

        glPopMatrix();
    }

    display_primitives(primitives, nTime, SlideWidthScale, SlideHeightScale);
    CHECK_GL_ERROR();
}

void OGLTransitionImpl::displayScene( double nTime, double SlideWidth, double SlideHeight, double DispWidth, double DispHeight )
{
    CHECK_GL_ERROR();
    const SceneObjects_t& rSceneObjects(maScene.getSceneObjects());
    glEnable(GL_TEXTURE_2D);
    for(size_t i(0); i != rSceneObjects.size(); ++i)
        rSceneObjects[i]->display(nTime, SlideWidth, SlideHeight, DispWidth, DispHeight);
    CHECK_GL_ERROR();
}

void Primitive::display(double nTime, double WidthScale, double HeightScale, int first) const
{
    CHECK_GL_ERROR();
    glPushMatrix();

    CHECK_GL_ERROR();
    applyOperations( nTime, WidthScale, HeightScale );

    CHECK_GL_ERROR();
    glDrawArrays( GL_TRIANGLES, first, Vertices.size() );

    CHECK_GL_ERROR();
    glPopMatrix();
    CHECK_GL_ERROR();
}

void Primitive::applyOperations(double nTime, double WidthScale, double HeightScale) const
{
    glm::mat4 matrix;
    for(size_t i(0); i < Operations.size(); ++i)
        Operations[i]->interpolate(matrix, nTime, WidthScale, HeightScale);
    matrix = glm::scale(matrix, glm::vec3(WidthScale, HeightScale, 1));
    CHECK_GL_ERROR();
    // TODO: replace that with an uniform upload instead.
    glMultMatrixf(glm::value_ptr(matrix));
    CHECK_GL_ERROR();
}

void SceneObject::display(double nTime, double /* SlideWidth */, double /* SlideHeight */, double DispWidth, double DispHeight ) const
{
    // fixme: allow various model spaces, now we make it so that
    // it is regular -1,-1 to 1,1, where the whole display fits in
    CHECK_GL_ERROR();
    glPushMatrix();
    CHECK_GL_ERROR();
    glm::mat4 matrix;
    if (DispHeight > DispWidth)
        matrix = glm::scale(matrix, glm::vec3(DispHeight/DispWidth, 1, 1));
    else
        matrix = glm::scale(matrix, glm::vec3(1, DispWidth/DispHeight, 1));
    glMultMatrixf(glm::value_ptr(matrix));
    CHECK_GL_ERROR();
    display_primitives(maPrimitives, nTime, 1, 1);
    CHECK_GL_ERROR();
    glPopMatrix();
    CHECK_GL_ERROR();
}

void SceneObject::pushPrimitive(const Primitive &p)
{
    maPrimitives.push_back(p);
}

SceneObject::SceneObject()
    : maPrimitives()
{
}

SceneObject::~SceneObject()
{
}

Iris::Iris()
    : SceneObject()
    , maTexture(0)
{
}

void Iris::display(double nTime, double SlideWidth, double SlideHeight, double DispWidth, double DispHeight ) const
{
    glBindTexture(GL_TEXTURE_2D, maTexture);
    CHECK_GL_ERROR();
    SceneObject::display(nTime, SlideWidth, SlideHeight, DispWidth, DispHeight);
}

void Iris::prepare()
{
    CHECK_GL_ERROR();
    static const GLubyte img[3] = { 80, 80, 80 };

    glGenTextures(1, &maTexture);
    glBindTexture(GL_TEXTURE_2D, maTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    CHECK_GL_ERROR();
}

void Iris::finish()
{
    CHECK_GL_ERROR();
    glDeleteTextures(1, &maTexture);
    CHECK_GL_ERROR();
}

namespace
{

class SimpleTransition : public OGLTransitionImpl
{
public:
    SimpleTransition()
        : OGLTransitionImpl()
    {
    }

    SimpleTransition(const TransitionScene& rScene, const TransitionSettings& rSettings)
        : OGLTransitionImpl(rScene, rSettings)
    {
    }
};

std::shared_ptr<OGLTransitionImpl>
makeSimpleTransition()
{
    return std::make_shared<SimpleTransition>();
}

std::shared_ptr<OGLTransitionImpl>
makeSimpleTransition(
        const Primitives_t& rLeavingSlidePrimitives,
        const Primitives_t& rEnteringSlidePrimitives,
        const Operations_t& rOverallOperations,
        const SceneObjects_t& rSceneObjects,
        const TransitionSettings& rSettings = TransitionSettings())
{
    return std::make_shared<SimpleTransition>(
            TransitionScene(rLeavingSlidePrimitives, rEnteringSlidePrimitives, rOverallOperations, rSceneObjects),
            rSettings);
}

std::shared_ptr<OGLTransitionImpl>
makeSimpleTransition(
        const Primitives_t& rLeavingSlidePrimitives,
        const Primitives_t& rEnteringSlidePrimitives,
        const Operations_t& rOverallOperations,
        const TransitionSettings& rSettings = TransitionSettings())
{
    return makeSimpleTransition(rLeavingSlidePrimitives, rEnteringSlidePrimitives, rOverallOperations, SceneObjects_t(), rSettings);
}

std::shared_ptr<OGLTransitionImpl>
makeSimpleTransition(
        const Primitives_t& rLeavingSlidePrimitives,
        const Primitives_t& rEnteringSlidePrimitives,
        const SceneObjects_t& rSceneObjects,
        const TransitionSettings& rSettings = TransitionSettings())
{
    return makeSimpleTransition(rLeavingSlidePrimitives, rEnteringSlidePrimitives, Operations_t(), rSceneObjects, rSettings);
}

std::shared_ptr<OGLTransitionImpl>
makeSimpleTransition(
        const Primitives_t& rLeavingSlidePrimitives,
        const Primitives_t& rEnteringSlidePrimitives,
        const TransitionSettings& rSettings = TransitionSettings())
{
    return makeSimpleTransition(rLeavingSlidePrimitives, rEnteringSlidePrimitives, Operations_t(), SceneObjects_t(), rSettings);
}

}

std::shared_ptr<OGLTransitionImpl> makeOutsideCubeFaceToLeft()
{
    Primitive Slide;

    Slide.pushTriangle(glm::vec2(0,0),glm::vec2(1,0),glm::vec2(0,1));
    Slide.pushTriangle(glm::vec2(1,0),glm::vec2(0,1),glm::vec2(1,1));

    Primitives_t aLeavingPrimitives;
    aLeavingPrimitives.push_back(Slide);

    Slide.Operations.push_back(makeRotateAndScaleDepthByWidth(glm::vec3(0,1,0),glm::vec3(0,0,-1),90,false,0.0,1.0));

    Primitives_t aEnteringPrimitives;
    aEnteringPrimitives.push_back(Slide);

    Operations_t aOperations;
    aOperations.push_back(makeRotateAndScaleDepthByWidth(glm::vec3(0,1,0),glm::vec3(0,0,-1),-90,true,0.0,1.0));

    return makeSimpleTransition(aLeavingPrimitives, aEnteringPrimitives, aOperations);
}

std::shared_ptr<OGLTransitionImpl> makeInsideCubeFaceToLeft()
{
    Primitive Slide;

    Slide.pushTriangle(glm::vec2(0,0),glm::vec2(1,0),glm::vec2(0,1));
    Slide.pushTriangle(glm::vec2(1,0),glm::vec2(0,1),glm::vec2(1,1));

    Primitives_t aLeavingPrimitives;
    aLeavingPrimitives.push_back(Slide);

    Slide.Operations.push_back(makeRotateAndScaleDepthByWidth(glm::vec3(0,1,0),glm::vec3(0,0,1),-90,false,0.0,1.0));

    Primitives_t aEnteringPrimitives;
    aEnteringPrimitives.push_back(Slide);

    Operations_t aOperations;
    aOperations.push_back(makeRotateAndScaleDepthByWidth(glm::vec3(0,1,0),glm::vec3(0,0,1),90,true,0.0,1.0));

    return makeSimpleTransition(aLeavingPrimitives, aEnteringPrimitives, aOperations);
}

std::shared_ptr<OGLTransitionImpl> makeFallLeaving()
{
    Primitive Slide;

    Slide.pushTriangle(glm::vec2(0,0),glm::vec2(1,0),glm::vec2(0,1));
    Slide.pushTriangle(glm::vec2(1,0),glm::vec2(0,1),glm::vec2(1,1));

    Primitives_t aEnteringPrimitives;
    aEnteringPrimitives.push_back(Slide);

    Slide.Operations.push_back(makeRotateAndScaleDepthByWidth(glm::vec3(1,0,0),glm::vec3(0,-1,0), 90,true,0.0,1.0));
    Primitives_t aLeavingPrimitives;
    aLeavingPrimitives.push_back(Slide);

    TransitionSettings aSettings;
    aSettings.mbUseMipMapEntering = false;

    return makeSimpleTransition(aLeavingPrimitives, aEnteringPrimitives, aSettings);
}

std::shared_ptr<OGLTransitionImpl> makeTurnAround()
{
    Primitive Slide;

    TransitionSettings aSettings;
    aSettings.mbReflectSlides = true;

    Slide.pushTriangle(glm::vec2(0,0),glm::vec2(1,0),glm::vec2(0,1));
    Slide.pushTriangle(glm::vec2(1,0),glm::vec2(0,1),glm::vec2(1,1));
    Primitives_t aLeavingPrimitives;
    aLeavingPrimitives.push_back(Slide);

    Slide.Operations.push_back(makeRotateAndScaleDepthByWidth(glm::vec3(0,1,0),glm::vec3(0,0,0),-180,false,0.0,1.0));
    Primitives_t aEnteringPrimitives;
    aEnteringPrimitives.push_back(Slide);

    Operations_t aOperations;
    aOperations.push_back(makeSTranslate(glm::vec3(0, 0, -1.5),true, 0, 0.5));
    aOperations.push_back(makeSTranslate(glm::vec3(0, 0, 1.5), true, 0.5, 1));
    aOperations.push_back(makeRotateAndScaleDepthByWidth(glm::vec3(0, 1, 0),glm::vec3(0, 0, 0), -180, true, 0.0, 1.0));

    return makeSimpleTransition(aLeavingPrimitives, aEnteringPrimitives, aOperations, aSettings);
}

std::shared_ptr<OGLTransitionImpl> makeTurnDown()
{
    Primitive Slide;

    Slide.pushTriangle(glm::vec2(0,0),glm::vec2(1,0),glm::vec2(0,1));
    Slide.pushTriangle(glm::vec2(1,0),glm::vec2(0,1),glm::vec2(1,1));
    Primitives_t aLeavingPrimitives;
    aLeavingPrimitives.push_back(Slide);

    Slide.Operations.push_back(makeSTranslate(glm::vec3(0, 0, 0.0001), false, -1.0, 0.0));
    Slide.Operations.push_back(makeSRotate (glm::vec3(0, 0, 1), glm::vec3(-1, 1, 0), -90, true, 0.0, 1.0));
    Slide.Operations.push_back(makeSRotate (glm::vec3(0, 0, 1), glm::vec3(-1, 1, 0), 90, false, -1.0, 0.0));
    Primitives_t aEnteringPrimitives;
    aEnteringPrimitives.push_back(Slide);

    TransitionSettings aSettings;
    aSettings.mbUseMipMapLeaving = false;

    return makeSimpleTransition(aLeavingPrimitives, aEnteringPrimitives, aSettings);
}

std::shared_ptr<OGLTransitionImpl> makeIris()
{
    Primitive Slide;

    Slide.pushTriangle (glm::vec2 (0,0), glm::vec2 (1,0), glm::vec2 (0,1));
    Slide.pushTriangle (glm::vec2 (1,0), glm::vec2 (0,1), glm::vec2 (1,1));
    Primitives_t aEnteringPrimitives;
    aEnteringPrimitives.push_back (Slide);

    Slide.Operations.push_back (makeSTranslate (glm::vec3 (0, 0,  0.000001), false, -1, 0));
    Slide.Operations.push_back (makeSTranslate (glm::vec3 (0, 0, -0.000002), false, 0.5, 1));
    Primitives_t aLeavingPrimitives;
    aLeavingPrimitives.push_back (Slide);


    Primitive irisPart, part;
    int i, nSteps = 24, nParts = 7;
    double t = 1.0/nSteps, lx = 1, ly = 0, of=2.2, f=1.42;

    for (i=1; i<=nSteps; i++) {
        double x = cos ((3*2*M_PI*t)/nParts);
        double y = -sin ((3*2*M_PI*t)/nParts);
        double cx = (f*x + 1)/2;
        double cy = (f*y + 1)/2;
        double lcx = (f*lx + 1)/2;
        double lcy = (f*ly + 1)/2;
        double cxo = (of*x + 1)/2;
        double cyo = (of*y + 1)/2;
        double lcxo = (of*lx + 1)/2;
        double lcyo = (of*ly + 1)/2;
        irisPart.pushTriangle (glm::vec2 (lcx, lcy),
                               glm::vec2 (lcxo, lcyo),
                               glm::vec2 (cx, cy));
        irisPart.pushTriangle (glm::vec2 (cx, cy),
                               glm::vec2 (lcxo, lcyo),
                               glm::vec2 (cxo, cyo));
        lx = x;
        ly = y;
        t += 1.0/nSteps;
    }

    std::shared_ptr<Iris> pIris = std::make_shared<Iris>();
    double angle = 87;

    for (i = 0; i < nParts; i++) {
        irisPart.Operations.clear ();
        double rx, ry;

        rx = cos ((2*M_PI*i)/nParts);
        ry = sin ((2*M_PI*i)/nParts);
        irisPart.Operations.push_back (makeSRotate (glm::vec3(0, 0, 1), glm::vec3(rx, ry, 0),  angle, true, 0.0, 0.5));
        irisPart.Operations.push_back (makeSRotate (glm::vec3(0, 0, 1), glm::vec3(rx, ry, 0), -angle, true, 0.5, 1));
        if (i > 0) {
            irisPart.Operations.push_back (makeSTranslate (glm::vec3(rx, ry, 0),  false, -1, 0));
            irisPart.Operations.push_back (makeSRotate (glm::vec3(0, 0, 1), glm::vec3(0, 0, 0), i*360.0/nParts, false, -1, 0));
            irisPart.Operations.push_back (makeSTranslate (glm::vec3(-1, 0, 0),  false, -1, 0));
        }
        irisPart.Operations.push_back(makeSTranslate(glm::vec3(0, 0, 1), false, -2, 0.0));
        irisPart.Operations.push_back (makeSRotate (glm::vec3(1, .5, 0), glm::vec3(1, 0, 0), -30, false, -1, 0));
        pIris->pushPrimitive (irisPart);
    }

    SceneObjects_t aSceneObjects;
    aSceneObjects.push_back (pIris);

    TransitionSettings aSettings;
    aSettings.mbUseMipMapLeaving = aSettings.mbUseMipMapEntering = false;

    return makeSimpleTransition(aLeavingPrimitives, aEnteringPrimitives, aSceneObjects, aSettings);
}

namespace
{

class RochadeTransition : public OGLTransitionImpl
{
public:
    RochadeTransition(const TransitionScene& rScene, const TransitionSettings& rSettings)
        : OGLTransitionImpl(rScene, rSettings)
    {}

private:
    virtual void displaySlides_( double nTime, sal_Int32 glLeavingSlideTex, sal_Int32 glEnteringSlideTex, double SlideWidthScale, double SlideHeightScale ) override;
};

void RochadeTransition::displaySlides_( double nTime, sal_Int32 glLeavingSlideTex, sal_Int32 glEnteringSlideTex, double SlideWidthScale, double SlideHeightScale )
{
    applyOverallOperations( nTime, SlideWidthScale, SlideHeightScale );

    glEnable(GL_TEXTURE_2D);

    if( nTime > .5) {
        displaySlide( nTime, glLeavingSlideTex, getScene().getLeavingSlide(), SlideWidthScale, SlideHeightScale );
        displaySlide( nTime, glEnteringSlideTex, getScene().getEnteringSlide(), SlideWidthScale, SlideHeightScale );
    } else {
        displaySlide( nTime, glEnteringSlideTex, getScene().getEnteringSlide(), SlideWidthScale, SlideHeightScale );
        displaySlide( nTime, glLeavingSlideTex, getScene().getLeavingSlide(), SlideWidthScale, SlideHeightScale );
    }
}

std::shared_ptr<OGLTransitionImpl>
makeRochadeTransition(
        const Primitives_t& rLeavingSlidePrimitives,
        const Primitives_t& rEnteringSlidePrimitives,
        const TransitionSettings& rSettings)
{
    return std::make_shared<RochadeTransition>(
            TransitionScene(rLeavingSlidePrimitives, rEnteringSlidePrimitives),
            rSettings)
        ;

}
}

std::shared_ptr<OGLTransitionImpl> makeRochade()
{
    Primitive Slide;

    TransitionSettings aSettings;
    aSettings.mbReflectSlides = true;

    double w, h;

    w = 2.2;
    h = 10;

    Slide.pushTriangle(glm::vec2(0,0),glm::vec2(1,0),glm::vec2(0,1));
    Slide.pushTriangle(glm::vec2(1,0),glm::vec2(0,1),glm::vec2(1,1));

    Slide.Operations.push_back(makeSEllipseTranslate(w, h, 0.25, -0.25, true, 0, 1));
    Slide.Operations.push_back(makeRotateAndScaleDepthByWidth(glm::vec3(0,1,0),glm::vec3(0,0,0), -45, true, 0, 1));
    Primitives_t aLeavingSlide;
    aLeavingSlide.push_back(Slide);

    Slide.Operations.clear();
    Slide.Operations.push_back(makeSEllipseTranslate(w, h, 0.75, 0.25, true, 0, 1));
    Slide.Operations.push_back(makeSTranslate(glm::vec3(0, 0, -h), false, -1, 0));
    Slide.Operations.push_back(makeRotateAndScaleDepthByWidth(glm::vec3(0,1,0),glm::vec3(0,0,0), -45, true, 0, 1));
    Slide.Operations.push_back(makeRotateAndScaleDepthByWidth(glm::vec3(0,1,0),glm::vec3(0,0,0), 45, false, -1, 0));
    Primitives_t aEnteringSlide;
    aEnteringSlide.push_back(Slide);

    return makeRochadeTransition(aLeavingSlide, aEnteringSlide, aSettings);
}

inline double randFromNeg1to1()
{
    return comphelper::rng::uniform_real_distribution(-1.0, std::nextafter(1.0, DBL_MAX));
}

// TODO(Q3): extract to basegfx
inline glm::vec3 randNormVectorInXYPlane()
{
    glm::vec3 toReturn(randFromNeg1to1(),randFromNeg1to1(),0.0);
    return glm::normalize(toReturn);
}

template<typename T>
T clamp(const T& rIn)
{
    return glm::clamp(rIn, T(-1.0), T(1.0));
}

std::shared_ptr<OGLTransitionImpl> makeRevolvingCircles( sal_uInt16 nCircles , sal_uInt16 nPointsOnCircles )
{
    double dAngle(2*3.1415926/static_cast<double>( nPointsOnCircles ));
    if(nCircles < 2 || nPointsOnCircles < 4)
    {
        makeNByMTileFlip(1,1);
        return makeSimpleTransition();
    }
    float Radius(1.0/static_cast<double>( nCircles ));
    float dRadius(Radius);
    float LastRadius(0.0);
    float NextRadius(2*Radius);

    /// now we know there is at least two circles
    /// the first will always be a full circle
    /// the last will always be the outer shell of the slide with a circle hole

    //add the full circle
    std::vector<glm::vec2> unScaledTexCoords;
    float TempAngle(0.0);
    for(unsigned int Point(0); Point < nPointsOnCircles; ++Point)
    {
        unScaledTexCoords.push_back( glm::vec2( cos(TempAngle - 3.1415926/2.0) , sin(TempAngle- 3.1415926/2.0) ) );

        TempAngle += dAngle;
    }

    Primitives_t aLeavingSlide;
    Primitives_t aEnteringSlide;
    {
        Primitive EnteringSlide;
        Primitive LeavingSlide;
        for(int Point(0); Point + 1 < nPointsOnCircles; ++Point)
        {
            EnteringSlide.pushTriangle( glm::vec2( 0.5 , 0.5 ) , Radius * unScaledTexCoords[ Point + 1 ] / 2.0f + glm::vec2( 0.5 , 0.5 ) , Radius * unScaledTexCoords[ Point ] / 2.0f + glm::vec2( 0.5 , 0.5 ) );
            LeavingSlide.pushTriangle( glm::vec2( 0.5 , 0.5 ) , Radius * unScaledTexCoords[ Point + 1 ] / 2.0f + glm::vec2( 0.5 , 0.5 ) , Radius * unScaledTexCoords[ Point ] / 2.0f + glm::vec2( 0.5, 0.5) );
        }
        EnteringSlide.pushTriangle( glm::vec2(0.5,0.5) , Radius * unScaledTexCoords[ 0 ] / 2.0f + glm::vec2( 0.5 , 0.5 ) , Radius * unScaledTexCoords[ nPointsOnCircles - 1 ] / 2.0f + glm::vec2( 0.5 , 0.5 ) );
        LeavingSlide.pushTriangle( glm::vec2(0.5,0.5) , Radius*unScaledTexCoords[0]/2.0f + glm::vec2(0.5,0.5) , Radius*unScaledTexCoords[nPointsOnCircles - 1]/2.0f + glm::vec2(0.5,0.5) );

        glm::vec3 axis(randNormVectorInXYPlane());
        EnteringSlide.Operations.push_back( makeSRotate( axis , glm::vec3(0,0,0) , 180, true, Radius/2.0 , (NextRadius + 1)/2.0 ) );
        LeavingSlide.Operations.push_back( makeSRotate( axis , glm::vec3(0,0,0) , 180, true, Radius/2.0 , (NextRadius + 1)/2.0 ) );
        EnteringSlide.Operations.push_back( makeSRotate( axis , glm::vec3(0,0,0) , -180, false,0.0,1.0) );

        aEnteringSlide.push_back(EnteringSlide);
        aLeavingSlide.push_back(LeavingSlide);
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
            EnteringSlide.pushTriangle(Radius*unScaledTexCoords[Side]/2.0f + glm::vec2(0.5,0.5) , LastRadius*unScaledTexCoords[Side]/2.0f + glm::vec2(0.5,0.5) , LastRadius*unScaledTexCoords[Side + 1]/2.0f + glm::vec2(0.5,0.5) );
            EnteringSlide.pushTriangle(Radius*unScaledTexCoords[Side]/2.0f + glm::vec2(0.5,0.5) , LastRadius*unScaledTexCoords[Side + 1]/2.0f + glm::vec2(0.5,0.5) , Radius*unScaledTexCoords[Side + 1]/2.0f + glm::vec2(0.5,0.5) );

            LeavingSlide.pushTriangle(Radius*unScaledTexCoords[Side]/2.0f + glm::vec2(0.5,0.5) , LastRadius*unScaledTexCoords[Side]/2.0f + glm::vec2(0.5,0.5) , LastRadius*unScaledTexCoords[Side + 1]/2.0f + glm::vec2(0.5,0.5) );
            LeavingSlide.pushTriangle(Radius*unScaledTexCoords[Side]/2.0f + glm::vec2(0.5,0.5) , LastRadius*unScaledTexCoords[Side + 1]/2.0f + glm::vec2(0.5,0.5) , Radius*unScaledTexCoords[Side + 1]/2.0f + glm::vec2(0.5,0.5) );
        }

        EnteringSlide.pushTriangle(Radius*unScaledTexCoords[nPointsOnCircles - 1]/2.0f + glm::vec2(0.5,0.5) , LastRadius*unScaledTexCoords[nPointsOnCircles - 1]/2.0f + glm::vec2(0.5,0.5) , LastRadius*unScaledTexCoords[0]/2.0f + glm::vec2(0.5,0.5) );
        EnteringSlide.pushTriangle(Radius*unScaledTexCoords[nPointsOnCircles - 1]/2.0f + glm::vec2(0.5,0.5) , LastRadius*unScaledTexCoords[0]/2.0f + glm::vec2(0.5,0.5) , Radius*unScaledTexCoords[0]/2.0f + glm::vec2(0.5,0.5) );

        LeavingSlide.pushTriangle(Radius*unScaledTexCoords[nPointsOnCircles - 1]/2.0f + glm::vec2(0.5,0.5) , LastRadius*unScaledTexCoords[nPointsOnCircles - 1]/2.0f + glm::vec2(0.5,0.5) , LastRadius*unScaledTexCoords[0]/2.0f + glm::vec2(0.5,0.5) );
        LeavingSlide.pushTriangle(Radius*unScaledTexCoords[nPointsOnCircles - 1]/2.0f + glm::vec2(0.5,0.5) , LastRadius*unScaledTexCoords[0]/2.0f + glm::vec2(0.5,0.5) , Radius*unScaledTexCoords[0]/2.0f + glm::vec2(0.5,0.5) );

        glm::vec3 axis(randNormVectorInXYPlane());
        EnteringSlide.Operations.push_back( makeSRotate( axis , glm::vec3(0,0,0) , 180, true, Radius/2.0 , (NextRadius + 1)/2.0 ) );
        LeavingSlide.Operations.push_back( makeSRotate( axis , glm::vec3(0,0,0) , 180, true, Radius/2.0 , (NextRadius + 1)/2.0 ) );
        EnteringSlide.Operations.push_back( makeSRotate( axis , glm::vec3(0,0,0) , -180, false,0.0,1.0) );

        aEnteringSlide.push_back(EnteringSlide);
        aLeavingSlide.push_back(LeavingSlide);

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

            EnteringSlide.pushTriangle(clamp(Radius*unScaledTexCoords[Side])/2.0f + glm::vec2(0.5,0.5) , LastRadius*unScaledTexCoords[Side]/2.0f + glm::vec2(0.5,0.5) , LastRadius*unScaledTexCoords[Side + 1]/2.0f + glm::vec2(0.5,0.5) );
            EnteringSlide.pushTriangle(clamp(Radius*unScaledTexCoords[Side])/2.0f + glm::vec2(0.5,0.5) , LastRadius*unScaledTexCoords[Side + 1]/2.0f + glm::vec2(0.5,0.5) , clamp(Radius*unScaledTexCoords[Side + 1])/2.0f + glm::vec2(0.5,0.5) );

            LeavingSlide.pushTriangle(clamp(Radius*unScaledTexCoords[Side])/2.0f + glm::vec2(0.5,0.5) , LastRadius*unScaledTexCoords[Side]/2.0f + glm::vec2(0.5,0.5) , LastRadius*unScaledTexCoords[Side + 1]/2.0f + glm::vec2(0.5,0.5) );
            LeavingSlide.pushTriangle(clamp(Radius*unScaledTexCoords[Side])/2.0f + glm::vec2(0.5,0.5) , LastRadius*unScaledTexCoords[Side + 1]/2.0f + glm::vec2(0.5,0.5) , clamp(Radius*unScaledTexCoords[Side + 1])/2.0f + glm::vec2(0.5,0.5) );
        }

        EnteringSlide.pushTriangle(clamp(Radius*unScaledTexCoords[nPointsOnCircles - 1])/2.0f + glm::vec2(0.5,0.5) , LastRadius*unScaledTexCoords[nPointsOnCircles - 1]/2.0f + glm::vec2(0.5,0.5) , LastRadius*unScaledTexCoords[0]/2.0f + glm::vec2(0.5,0.5) );
        EnteringSlide.pushTriangle(clamp(Radius*unScaledTexCoords[nPointsOnCircles - 1])/2.0f + glm::vec2(0.5,0.5) , LastRadius*unScaledTexCoords[0]/2.0f + glm::vec2(0.5,0.5) , clamp(Radius*unScaledTexCoords[0])/2.0f + glm::vec2(0.5,0.5) );

        LeavingSlide.pushTriangle(clamp(Radius*unScaledTexCoords[nPointsOnCircles - 1])/2.0f + glm::vec2(0.5,0.5) , LastRadius*unScaledTexCoords[nPointsOnCircles - 1]/2.0f + glm::vec2(0.5,0.5) , LastRadius*unScaledTexCoords[0]/2.0f + glm::vec2(0.5,0.5) );
        LeavingSlide.pushTriangle(clamp(Radius*unScaledTexCoords[nPointsOnCircles - 1])/2.0f + glm::vec2(0.5,0.5) , LastRadius*unScaledTexCoords[0]/2.0f + glm::vec2(0.5,0.5) , clamp(Radius*unScaledTexCoords[0])/2.0f + glm::vec2(0.5,0.5) );

        glm::vec3 axis(randNormVectorInXYPlane());
        EnteringSlide.Operations.push_back( makeSRotate( axis , glm::vec3(0,0,0) , 180, true, (LastRadius + dRadius)/2.0 , 1.0 ) );
        LeavingSlide.Operations.push_back( makeSRotate( axis , glm::vec3(0,0,0) , 180, true, (LastRadius + dRadius)/2.0 , 1.0 ) );
        EnteringSlide.Operations.push_back( makeSRotate( axis , glm::vec3(0,0,0) , -180, false,0.0,1.0) );

        aEnteringSlide.push_back(EnteringSlide);
        aLeavingSlide.push_back(LeavingSlide);
    }

    return makeSimpleTransition(aLeavingSlide, aEnteringSlide);
}

std::shared_ptr<OGLTransitionImpl> makeHelix( sal_uInt16 nRows )
{
    double invN(1.0/static_cast<double>(nRows));
    double iDn = 0.0;
    double iPDn = invN;
    Primitives_t aLeavingSlide;
    Primitives_t aEnteringSlide;
    for(unsigned int i(0); i < nRows; ++i)
    {
        Primitive Tile;

        Tile.pushTriangle(glm::vec2( 1.0 , iDn ) , glm::vec2( 0.0 , iDn ) , glm::vec2( 0.0 , iPDn ));

        Tile.pushTriangle(glm::vec2( 1.0 , iPDn ) , glm::vec2( 1.0 , iDn ) , glm::vec2( 0.0 , iPDn ));

        Tile.Operations.push_back( makeSRotate( glm::vec3( 0 , 1 , 0 ) , ( Tile.getVertex(1) + Tile.getVertex(3) )/2.0f , 180 ,
                                                true, std::min(std::max(static_cast<double>(i - nRows/2.0)*invN/2.0,0.0),1.0),
                                                std::min(std::max(static_cast<double>(i + nRows/2.0)*invN/2.0,0.0),1.0) ) );

        aLeavingSlide.push_back(Tile);

        Tile.Operations.push_back( makeSRotate( glm::vec3( 0 , 1 , 0 ) , ( Tile.getVertex(1) + Tile.getVertex(3) )/2.0f , -180 , false,0.0,1.0) );

        aEnteringSlide.push_back(Tile);

        iDn += invN;
        iPDn += invN;
    }

    return makeSimpleTransition(aLeavingSlide, aEnteringSlide);
}

float fdiv(int a, int b)
{
    return static_cast<float>(a)/b;
}

glm::vec2 vec(float x, float y, float nx, float ny)
{
    x = x < 0.0 ? 0.0 : x;
    x = x > nx  ? nx  : x;
    y = y < 0.0 ? 0.0 : y;
    y = y > ny  ? ny  : y;
    return glm::vec2(fdiv(x, nx), fdiv(y, ny));
}

std::shared_ptr<OGLTransitionImpl> makeNByMTileFlip( sal_uInt16 n, sal_uInt16 m )
{
    Primitives_t aLeavingSlide;
    Primitives_t aEnteringSlide;

    for (int x = 0; x < n; x++)
    {
        for (int y = 0; y < n; y++)
        {
            Primitive aTile;
            glm::vec2 x11 = vec(x,   y,   n, m);
            glm::vec2 x12 = vec(x,   y+1, n, m);
            glm::vec2 x21 = vec(x+1, y,   n, m);
            glm::vec2 x22 = vec(x+1, y+1, n, m);

            aTile.pushTriangle(x21, x11, x12);
            aTile.pushTriangle(x22, x21, x12);

            aTile.Operations.push_back(makeSRotate( glm::vec3(0 , 1, 0), (aTile.getVertex(1) + aTile.getVertex(3)) / 2.0f, 180 , true, x11.x * x11.y / 2.0f , ((x22.x * x22.y) + 1.0f) / 2.0f));
            aLeavingSlide.push_back(aTile);

            aTile.Operations.push_back(makeSRotate( glm::vec3(0 , 1, 0), (aTile.getVertex(1) + aTile.getVertex(3)) / 2.0f, -180, false, x11.x * x11.y / 2.0f , ((x22.x * x22.y) + 1.0f) / 2.0f));
            aEnteringSlide.push_back(aTile);
        }
    }

    return makeSimpleTransition(aLeavingSlide, aEnteringSlide);
}

SRotate::SRotate(const glm::vec3& Axis, const glm::vec3& Origin,
        double Angle, bool bInter, double T0, double T1):
    Operation(bInter, T0, T1),
    axis(Axis),
    origin(Origin),
    angle(Angle)
{
}

SScale::SScale(const glm::vec3& Scale, const glm::vec3& Origin,
        bool bInter, double T0, double T1):
    Operation(bInter, T0, T1),
    scale(Scale),
    origin(Origin)
{
}

RotateAndScaleDepthByWidth::RotateAndScaleDepthByWidth(const glm::vec3& Axis,
        const glm::vec3& Origin, double Angle, bool bInter, double T0, double T1):
    Operation(bInter, T0, T1),
    axis(Axis),
    origin(Origin),
    angle(Angle)
{
}

RotateAndScaleDepthByHeight::RotateAndScaleDepthByHeight(const glm::vec3& Axis,
        const glm::vec3& Origin, double Angle, bool bInter, double T0, double T1):
    Operation(bInter, T0, T1),
    axis(Axis),
    origin(Origin),
    angle(Angle)
{
}


STranslate::STranslate(const glm::vec3& Vector, bool bInter, double T0, double T1):
    Operation(bInter, T0, T1),
    vector(Vector)
{
}

std::shared_ptr<SRotate>
makeSRotate(const glm::vec3& Axis,const glm::vec3& Origin,double Angle,bool bInter, double T0, double T1)
{
    return std::make_shared<SRotate>(Axis, Origin, Angle, bInter, T0, T1);
}

std::shared_ptr<SScale>
makeSScale(const glm::vec3& Scale, const glm::vec3& Origin,bool bInter, double T0, double T1)
{
    return std::make_shared<SScale>(Scale, Origin, bInter, T0, T1);
}

std::shared_ptr<STranslate>
makeSTranslate(const glm::vec3& Vector,bool bInter, double T0, double T1)
{
    return std::make_shared<STranslate>(Vector, bInter, T0, T1);
}

std::shared_ptr<SEllipseTranslate>
makeSEllipseTranslate(double dWidth, double dHeight, double dStartPosition, double dEndPosition, bool bInter, double T0, double T1)
{
    return std::make_shared<SEllipseTranslate>(dWidth, dHeight, dStartPosition, dEndPosition, bInter, T0, T1);
}

std::shared_ptr<RotateAndScaleDepthByWidth>
makeRotateAndScaleDepthByWidth(const glm::vec3& Axis,const glm::vec3& Origin,double Angle,bool bInter, double T0, double T1)
{
    return std::make_shared<RotateAndScaleDepthByWidth>(Axis, Origin, Angle, bInter, T0, T1);
}

std::shared_ptr<RotateAndScaleDepthByHeight>
makeRotateAndScaleDepthByHeight(const glm::vec3& Axis,const glm::vec3& Origin,double Angle,bool bInter, double T0, double T1)
{
    return std::make_shared<RotateAndScaleDepthByHeight>(Axis, Origin, Angle, bInter, T0, T1);
}

inline double intervalInter(double t, double T0, double T1)
{
    return ( t - T0 ) / ( T1 - T0 );
}

void STranslate::interpolate(glm::mat4& matrix, double t, double SlideWidthScale, double SlideHeightScale) const
{
    if(t <= mnT0)
        return;
    if(!mbInterpolate || t > mnT1)
        t = mnT1;
    t = intervalInter(t,mnT0,mnT1);
    matrix = glm::translate(matrix, glm::vec3(SlideWidthScale*t*vector.x, SlideHeightScale*t*vector.y, t*vector.z));
}

void SRotate::interpolate(glm::mat4& matrix, double t, double SlideWidthScale, double SlideHeightScale) const
{
    if(t <= mnT0)
        return;
    if(!mbInterpolate || t > mnT1)
        t = mnT1;
    t = intervalInter(t,mnT0,mnT1);
    glm::vec3 translation_vector(SlideWidthScale*origin.x, SlideHeightScale*origin.y, origin.z);
    glm::vec3 scale_vector(SlideWidthScale, SlideHeightScale, 1);
    matrix = glm::translate(matrix, translation_vector);
    matrix = glm::scale(matrix, scale_vector);
    matrix = glm::rotate(matrix, static_cast<float>(t*angle), axis);
    matrix = glm::scale(matrix, 1.f / scale_vector);
    matrix = glm::translate(matrix, -translation_vector);
}

void SScale::interpolate(glm::mat4& matrix, double t, double SlideWidthScale, double SlideHeightScale) const
{
    if(t <= mnT0)
        return;
    if(!mbInterpolate || t > mnT1)
        t = mnT1;
    t = intervalInter(t,mnT0,mnT1);
    glm::vec3 translation_vector(SlideWidthScale*origin.x, SlideHeightScale*origin.y, origin.z);
    matrix = glm::translate(matrix, translation_vector);
    matrix = glm::scale(matrix, static_cast<float>(1 - t) + static_cast<float>(t) * scale);
    matrix = glm::translate(matrix, -translation_vector);
}

void RotateAndScaleDepthByWidth::interpolate(glm::mat4& matrix, double t, double SlideWidthScale, double SlideHeightScale) const
{
    if(t <= mnT0)
        return;
    if(!mbInterpolate || t > mnT1)
        t = mnT1;
    t = intervalInter(t,mnT0,mnT1);
    glm::vec3 translation_vector(SlideWidthScale*origin.x, SlideHeightScale*origin.y, SlideWidthScale*origin.z);
    matrix = glm::translate(matrix, translation_vector);
    matrix = glm::rotate(matrix, static_cast<float>(t*angle), axis);
    matrix = glm::translate(matrix, -translation_vector);
}

void RotateAndScaleDepthByHeight::interpolate(glm::mat4& matrix, double t, double SlideWidthScale, double SlideHeightScale) const
{
    if(t <= mnT0)
        return;
    if(!mbInterpolate || t > mnT1)
        t = mnT1;
    t = intervalInter(t,mnT0,mnT1);
    glm::vec3 translation_vector(SlideWidthScale*origin.x, SlideHeightScale*origin.y, SlideHeightScale*origin.z);
    matrix = glm::translate(matrix, translation_vector);
    matrix = glm::rotate(matrix, static_cast<float>(t*angle), axis);
    matrix = glm::translate(matrix, -translation_vector);
}

SEllipseTranslate::SEllipseTranslate(double dWidth, double dHeight, double dStartPosition,
        double dEndPosition, bool bInter, double T0, double T1):
    Operation(bInter, T0, T1)
{
    width = dWidth;
    height = dHeight;
    startPosition = dStartPosition;
    endPosition = dEndPosition;
}

void SEllipseTranslate::interpolate(glm::mat4& matrix, double t, double /* SlideWidthScale */, double /* SlideHeightScale */) const
{
    if(t <= mnT0)
        return;
    if(!mbInterpolate || t > mnT1)
        t = mnT1;
    t = intervalInter(t,mnT0,mnT1);

    double a1, a2, x, y;
    a1 = startPosition*2*M_PI;
    a2 = (startPosition + t*(endPosition - startPosition))*2*M_PI;
    x = width*(cos (a2) - cos (a1))/2;
    y = height*(sin (a2) - sin (a1))/2;

    matrix = glm::translate(matrix, glm::vec3(x, 0, y));
}

Primitive& Primitive::operator=(const Primitive& rvalue)
{
    Primitive aTmp(rvalue);
    swap(aTmp);
    return *this;
}

Primitive::Primitive(const Primitive& rvalue)
    : Operations(rvalue.Operations)
    , Vertices(rvalue.Vertices)
{
}

void Primitive::swap(Primitive& rOther)
{
    using std::swap;

    swap(Operations, rOther.Operations);
    swap(Vertices, rOther.Vertices);
}

void Primitive::pushTriangle(const glm::vec2& SlideLocation0,const glm::vec2& SlideLocation1,const glm::vec2& SlideLocation2)
{
    std::vector<glm::vec3> Verts;
    std::vector<glm::vec2> Texs;
    Verts.reserve(3);
    Texs.reserve(3);

    Verts.push_back(glm::vec3( 2*SlideLocation0.x - 1, -2*SlideLocation0.y + 1 , 0.0 ));
    Verts.push_back(glm::vec3( 2*SlideLocation1.x - 1, -2*SlideLocation1.y + 1 , 0.0 ));
    Verts.push_back(glm::vec3( 2*SlideLocation2.x - 1, -2*SlideLocation2.y + 1 , 0.0 ));

    //figure out if they're facing the correct way, and make them face the correct way.
    glm::vec3 Normal( glm::cross( Verts[0] - Verts[1] , Verts[1] - Verts[2] ) );
    if(Normal.z >= 0.0)//if the normal is facing us
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
        Verts.push_back(glm::vec3( 2*SlideLocation0.x - 1, -2*SlideLocation0.y + 1 , 0.0 ));
        Verts.push_back(glm::vec3( 2*SlideLocation2.x - 1, -2*SlideLocation2.y + 1 , 0.0 ));
        Verts.push_back(glm::vec3( 2*SlideLocation1.x - 1, -2*SlideLocation1.y + 1 , 0.0 ));
    }

    Vertices.push_back({Verts[0], glm::vec3(0, 0, 1), Texs[0]}); //all normals always face the screen when untransformed.
    Vertices.push_back({Verts[1], glm::vec3(0, 0, 1), Texs[1]}); //all normals always face the screen when untransformed.
    Vertices.push_back({Verts[2], glm::vec3(0, 0, 1), Texs[2]}); //all normals always face the screen when untransformed.
}

namespace
{

class DiamondTransition : public OGLTransitionImpl
{
public:
    DiamondTransition(const TransitionScene& rScene, const TransitionSettings& rSettings)
        : OGLTransitionImpl(rScene, rSettings)
        {}

private:
    virtual void prepare( double nTime, double SlideWidth, double SlideHeight, double DispWidth, double DispHeight ) override;
    // mmPrepare = &OGLTransitionImpl::prepareDiamond;
};

void DiamondTransition::prepare( double nTime, double /* SlideWidth */, double /* SlideHeight */, double /* DispWidth */, double /* DispHeight */ )
{
    Primitive Slide1, Slide2;

    Slide1.pushTriangle (glm::vec2 (0,0), glm::vec2 (1,0), glm::vec2 (0,1));
    Slide1.pushTriangle (glm::vec2 (1,0), glm::vec2 (0,1), glm::vec2 (1,1));
    Primitives_t aEnteringSlidePrimitives;
    aEnteringSlidePrimitives.push_back (Slide1);

    if( nTime >= 0.5 ) {
        double m = 1 - nTime;

        Slide2.pushTriangle (glm::vec2 (0,0), glm::vec2 (m,0), glm::vec2 (0,m));
        Slide2.pushTriangle (glm::vec2 (nTime,0), glm::vec2 (1,0), glm::vec2 (1,m));
        Slide2.pushTriangle (glm::vec2 (1,nTime), glm::vec2 (1,1), glm::vec2 (nTime,1));
        Slide2.pushTriangle (glm::vec2 (0,nTime), glm::vec2 (m,1), glm::vec2 (0,1));
    } else {
        double l = 0.5 - nTime;
        double h = 0.5 + nTime;

        Slide2.pushTriangle (glm::vec2 (0,0), glm::vec2 (1,0), glm::vec2 (0.5,l));
        Slide2.pushTriangle (glm::vec2 (0.5,l), glm::vec2 (1,0), glm::vec2 (h,0.5));
        Slide2.pushTriangle (glm::vec2 (1,0), glm::vec2 (1,1), glm::vec2 (h,0.5));
        Slide2.pushTriangle (glm::vec2 (h,0.5), glm::vec2 (1,1), glm::vec2 (0.5,h));
        Slide2.pushTriangle (glm::vec2 (0.5,h), glm::vec2 (1,1), glm::vec2 (0,1));
        Slide2.pushTriangle (glm::vec2 (l,0.5), glm::vec2 (0.5,h), glm::vec2 (0,1));
        Slide2.pushTriangle (glm::vec2 (0,0), glm::vec2 (l,0.5), glm::vec2 (0,1));
        Slide2.pushTriangle (glm::vec2 (0,0), glm::vec2 (0.5,l), glm::vec2 (l,0.5));
    }
    Slide2.Operations.push_back (makeSTranslate (glm::vec3 (0, 0, 0.00000001), false, -1, 0));
    Primitives_t aLeavingSlidePrimitives;
    aLeavingSlidePrimitives.push_back (Slide2);

    setScene(TransitionScene(aLeavingSlidePrimitives, aEnteringSlidePrimitives));
}

std::shared_ptr<OGLTransitionImpl>
makeDiamondTransition(const TransitionSettings& rSettings)
{
    return std::make_shared<DiamondTransition>(TransitionScene(), rSettings);
}

}

std::shared_ptr<OGLTransitionImpl> makeDiamond()
{
    TransitionSettings aSettings;
    aSettings.mbUseMipMapLeaving = aSettings.mbUseMipMapEntering = false;

    return makeDiamondTransition(aSettings);
}

std::shared_ptr<OGLTransitionImpl> makeVenetianBlinds( bool vertical, int parts )
{
    static double t30 = tan( M_PI/6.0 );
    double ln = 0;
    double p = 1.0/parts;

    Primitives_t aLeavingSlide;
    Primitives_t aEnteringSlide;
    for( int i=0; i<parts; i++ ) {
        Primitive Slide;
        double n = (i + 1)/(double)parts;
        if( vertical ) {
            Slide.pushTriangle (glm::vec2 (ln,0), glm::vec2 (n,0), glm::vec2 (ln,1));
            Slide.pushTriangle (glm::vec2 (n,0), glm::vec2 (ln,1), glm::vec2 (n,1));
            Slide.Operations.push_back(makeRotateAndScaleDepthByWidth(glm::vec3(0, 1, 0), glm::vec3(n + ln - 1, 0, -t30*p), -120, true, 0.0, 1.0));
        } else {
            Slide.pushTriangle (glm::vec2 (0,ln), glm::vec2 (1,ln), glm::vec2 (0,n));
            Slide.pushTriangle (glm::vec2 (1,ln), glm::vec2 (0,n), glm::vec2 (1,n));
            Slide.Operations.push_back(makeRotateAndScaleDepthByHeight(glm::vec3(1, 0, 0), glm::vec3(0, 1 - n - ln, -t30*p), -120, true, 0.0, 1.0));
        }
        aLeavingSlide.push_back (Slide);

        if( vertical ) {
            Slide.Operations.push_back(makeSRotate(glm::vec3(0, 1, 0), glm::vec3(2*n - 1, 0, 0), -60, false, -1, 0));
            Slide.Operations.push_back(makeSRotate(glm::vec3(0, 1, 0), glm::vec3(n + ln - 1, 0, 0), 180, false, -1, 0));
        } else {
            Slide.Operations.push_back(makeSRotate(glm::vec3(1, 0, 0), glm::vec3(0, 1 - 2*n, 0), -60, false, -1, 0));
            Slide.Operations.push_back(makeSRotate(glm::vec3(1, 0, 0), glm::vec3(0, 1 - n - ln, 0), 180, false, -1, 0));
        }
        aEnteringSlide.push_back (Slide);
        ln = n;
    }

    return makeSimpleTransition(aLeavingSlide, aEnteringSlide);
}

namespace
{

class FadeSmoothlyTransition : public OGLTransitionImpl
{
public:
    FadeSmoothlyTransition(const TransitionScene& rScene, const TransitionSettings& rSettings)
        : OGLTransitionImpl(rScene, rSettings)
    {}

private:
    virtual void displaySlides_( double nTime, sal_Int32 glLeavingSlideTex, sal_Int32 glEnteringSlideTex, double SlideWidthScale, double SlideHeightScale ) override;
};

void FadeSmoothlyTransition::displaySlides_( double nTime, sal_Int32 glLeavingSlideTex, sal_Int32 glEnteringSlideTex, double SlideWidthScale, double SlideHeightScale )
{
    CHECK_GL_ERROR();
    applyOverallOperations( nTime, SlideWidthScale, SlideHeightScale );

    CHECK_GL_ERROR();
    glDisable(GL_DEPTH_TEST);

    CHECK_GL_ERROR();
    displaySlide( nTime, glLeavingSlideTex, getScene().getLeavingSlide(), SlideWidthScale, SlideHeightScale );
    CHECK_GL_ERROR();

    CHECK_GL_ERROR();
    glDisable(GL_LIGHTING);
    CHECK_GL_ERROR();
    glEnable(GL_BLEND);
    CHECK_GL_ERROR();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    CHECK_GL_ERROR();
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    CHECK_GL_ERROR();
    glColor4f( 1, 1, 1, nTime );
    CHECK_GL_ERROR();
    displaySlide( nTime, glEnteringSlideTex, getScene().getEnteringSlide(), SlideWidthScale, SlideHeightScale );
    CHECK_GL_ERROR();
    glDisable(GL_BLEND);
    CHECK_GL_ERROR();
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    CHECK_GL_ERROR();
    glEnable(GL_LIGHTING);
    CHECK_GL_ERROR();

    glEnable(GL_DEPTH_TEST);
    CHECK_GL_ERROR();
}

std::shared_ptr<OGLTransitionImpl>
makeFadeSmoothlyTransition(
        const Primitives_t& rLeavingSlidePrimitives,
        const Primitives_t& rEnteringSlidePrimitives,
        const TransitionSettings& rSettings)
{
    return std::make_shared<FadeSmoothlyTransition>(
            TransitionScene(rLeavingSlidePrimitives, rEnteringSlidePrimitives),
            rSettings)
        ;
}

}

std::shared_ptr<OGLTransitionImpl> makeFadeSmoothly()
{
    Primitive Slide;

    Slide.pushTriangle (glm::vec2 (0,0), glm::vec2 (1,0), glm::vec2 (0,1));
    Slide.pushTriangle (glm::vec2 (1,0), glm::vec2 (0,1), glm::vec2 (1,1));
    Primitives_t aLeavingSlide;
    aLeavingSlide.push_back (Slide);
    Primitives_t aEnteringSlide;
    aEnteringSlide.push_back (Slide);

    TransitionSettings aSettings;
    aSettings.mbUseMipMapLeaving = aSettings.mbUseMipMapEntering = false;

    return makeFadeSmoothlyTransition(aLeavingSlide, aEnteringSlide, aSettings);
}

namespace
{

class FadeThroughBlackTransition : public OGLTransitionImpl
{
public:
    FadeThroughBlackTransition(const TransitionScene& rScene, const TransitionSettings& rSettings)
        : OGLTransitionImpl(rScene, rSettings)
    {}

private:
    virtual void displaySlides_( double nTime, sal_Int32 glLeavingSlideTex, sal_Int32 glEnteringSlideTex, double SlideWidthScale, double SlideHeightScale ) override;
};

void FadeThroughBlackTransition::displaySlides_( double nTime, sal_Int32 glLeavingSlideTex, sal_Int32 glEnteringSlideTex, double SlideWidthScale, double SlideHeightScale )
{
    CHECK_GL_ERROR();
    applyOverallOperations( nTime, SlideWidthScale, SlideHeightScale );

    glDisable(GL_DEPTH_TEST);

    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    if( nTime < 0.5 ) {
        glColor4f( 1, 1, 1, 1 - nTime*2 );
        displaySlide( nTime, glLeavingSlideTex, getScene().getLeavingSlide(), SlideWidthScale, SlideHeightScale );
    } else {
        glColor4f( 1, 1, 1, (nTime - 0.5)*2 );
        displaySlide( nTime, glEnteringSlideTex, getScene().getEnteringSlide(), SlideWidthScale, SlideHeightScale );
    }
    glDisable(GL_BLEND);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glEnable(GL_LIGHTING);

    glEnable(GL_DEPTH_TEST);
    CHECK_GL_ERROR();
}

std::shared_ptr<OGLTransitionImpl>
makeFadeThroughBlackTransition(
        const Primitives_t& rLeavingSlidePrimitives,
        const Primitives_t& rEnteringSlidePrimitives,
        const TransitionSettings& rSettings)
{
    return std::make_shared<FadeThroughBlackTransition>(
            TransitionScene(rLeavingSlidePrimitives, rEnteringSlidePrimitives),
            rSettings)
        ;
}

}

std::shared_ptr<OGLTransitionImpl> makeFadeThroughBlack()
{
    Primitive Slide;

    Slide.pushTriangle (glm::vec2 (0,0), glm::vec2 (1,0), glm::vec2 (0,1));
    Slide.pushTriangle (glm::vec2 (1,0), glm::vec2 (0,1), glm::vec2 (1,1));
    Primitives_t aLeavingSlide;
    aLeavingSlide.push_back (Slide);
    Primitives_t aEnteringSlide;
    aEnteringSlide.push_back (Slide);

    TransitionSettings aSettings;
    aSettings.mbUseMipMapLeaving = aSettings.mbUseMipMapEntering = false;

    return makeFadeThroughBlackTransition(aLeavingSlide, aEnteringSlide, aSettings);
}

namespace
{

class ShaderTransition : public OGLTransitionImpl
{
protected:
    ShaderTransition(const TransitionScene& rScene, const TransitionSettings& rSettings)
        : OGLTransitionImpl(rScene, rSettings)
        , m_nProgramObject(0)
        , m_nHelperTexture(0)
    {}

private:
    virtual void displaySlides_( double nTime, sal_Int32 glLeavingSlideTex, sal_Int32 glEnteringSlideTex, double SlideWidthScale, double SlideHeightScale ) override;
    virtual void prepareTransition( sal_Int32 glLeavingSlideTex, sal_Int32 glEnteringSlideTex ) override;
    virtual void finishTransition() override;
    virtual GLuint makeShader() = 0;

    void impl_preparePermShader();

private:
    /** GLSL program object
     */
    GLuint m_nProgramObject;

    /** various data */
    GLuint m_nHelperTexture;
};

void ShaderTransition::displaySlides_( double nTime, sal_Int32 glLeavingSlideTex, sal_Int32 glEnteringSlideTex,
                                              double SlideWidthScale, double SlideHeightScale )
{
    CHECK_GL_ERROR();
    applyOverallOperations( nTime, SlideWidthScale, SlideHeightScale );

    if( m_nProgramObject ) {
        GLint location = glGetUniformLocation( m_nProgramObject, "time" );
        if( location != -1 ) {
            glUniform1f( location, nTime );
        }
    }

    glActiveTexture( GL_TEXTURE2 );
    glBindTexture( GL_TEXTURE_2D, glEnteringSlideTex );
    glActiveTexture( GL_TEXTURE0 );

    displaySlide( nTime, glLeavingSlideTex, getScene().getLeavingSlide(), SlideWidthScale, SlideHeightScale );
    CHECK_GL_ERROR();
}

void ShaderTransition::prepareTransition( sal_Int32 /* glLeavingSlideTex */, sal_Int32 /* glEnteringSlideTex */ )
{
    m_nProgramObject = makeShader();

    impl_preparePermShader();
}

void ShaderTransition::finishTransition()
{
    CHECK_GL_ERROR();
    if( m_nProgramObject ) {
        glDeleteProgram( m_nProgramObject );
        m_nProgramObject = 0;
    }
    if ( m_nHelperTexture )
    {
        glDeleteTextures( 1, &m_nHelperTexture );
        m_nHelperTexture = 0;
    }
    CHECK_GL_ERROR();
}

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
    CHECK_GL_ERROR();
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
    CHECK_GL_ERROR();
}

void ShaderTransition::impl_preparePermShader()
{
    CHECK_GL_ERROR();
    if( m_nProgramObject ) {
        glUseProgram( m_nProgramObject );
        CHECK_GL_ERROR();

        GLint location = glGetUniformLocation( m_nProgramObject, "leavingSlideTexture" );
        if( location != -1 ) {
            glUniform1i( location, 0 );  // texture unit 0
            CHECK_GL_ERROR();
        }

        location = glGetUniformLocation( m_nProgramObject, "permTexture" );
        if( location != -1 ) {
            glActiveTexture(GL_TEXTURE1);
            CHECK_GL_ERROR();
            if( !m_nHelperTexture )
                initPermTexture( &m_nHelperTexture );

            glActiveTexture(GL_TEXTURE0);
            CHECK_GL_ERROR();

            glUniform1i( location, 1 );  // texture unit 1
            CHECK_GL_ERROR();
        }

        location = glGetUniformLocation( m_nProgramObject, "enteringSlideTexture" );
        if( location != -1 ) {
            glUniform1i( location, 2 );  // texture unit 2
            CHECK_GL_ERROR();
        }
    }
    CHECK_GL_ERROR();
}

}

namespace
{

class StaticNoiseTransition : public ShaderTransition
{
public:
    StaticNoiseTransition(const TransitionScene& rScene, const TransitionSettings& rSettings)
        : ShaderTransition(rScene, rSettings)
    {}

private:
    virtual GLuint makeShader() override;
};

GLuint StaticNoiseTransition::makeShader()
{
    return OpenGLHelper::LoadShaders( "basicVertexShader", "staticFragmentShader" );
}

std::shared_ptr<OGLTransitionImpl>
makeStaticNoiseTransition(
        const Primitives_t& rLeavingSlidePrimitives,
        const Primitives_t& rEnteringSlidePrimitives,
        const TransitionSettings& rSettings)
{
    return std::make_shared<StaticNoiseTransition>(
            TransitionScene(rLeavingSlidePrimitives, rEnteringSlidePrimitives),
            rSettings)
        ;
}

}

std::shared_ptr<OGLTransitionImpl> makeStatic()
{
    Primitive Slide;

    Slide.pushTriangle (glm::vec2 (0,0), glm::vec2 (1,0), glm::vec2 (0,1));
    Slide.pushTriangle (glm::vec2 (1,0), glm::vec2 (0,1), glm::vec2 (1,1));
    Primitives_t aLeavingSlide;
    aLeavingSlide.push_back (Slide);
    Primitives_t aEnteringSlide;
    aEnteringSlide.push_back (Slide);

    TransitionSettings aSettings;
    aSettings.mbUseMipMapLeaving = aSettings.mbUseMipMapEntering = false;
    aSettings.mnRequiredGLVersion = 2.0;

    return makeStaticNoiseTransition(aLeavingSlide, aEnteringSlide, aSettings);
}

namespace
{

class DissolveTransition : public ShaderTransition
{
public:
    DissolveTransition(const TransitionScene& rScene, const TransitionSettings& rSettings)
        : ShaderTransition(rScene, rSettings)
    {}

private:
    virtual GLuint makeShader() override;
};

GLuint DissolveTransition::makeShader()
{
    return OpenGLHelper::LoadShaders( "basicVertexShader", "dissolveFragmentShader" );
}

std::shared_ptr<OGLTransitionImpl>
makeDissolveTransition(
        const Primitives_t& rLeavingSlidePrimitives,
        const Primitives_t& rEnteringSlidePrimitives,
        const TransitionSettings& rSettings)
{
    return std::make_shared<DissolveTransition>(
            TransitionScene(rLeavingSlidePrimitives, rEnteringSlidePrimitives),
            rSettings)
        ;
}

}

std::shared_ptr<OGLTransitionImpl> makeDissolve()
{
    Primitive Slide;

    Slide.pushTriangle (glm::vec2 (0,0), glm::vec2 (1,0), glm::vec2 (0,1));
    Slide.pushTriangle (glm::vec2 (1,0), glm::vec2 (0,1), glm::vec2 (1,1));
    Primitives_t aLeavingSlide;
    aLeavingSlide.push_back (Slide);
    Primitives_t aEnteringSlide;
    aEnteringSlide.push_back (Slide);

    TransitionSettings aSettings;
    aSettings.mbUseMipMapLeaving = aSettings.mbUseMipMapEntering = false;
    aSettings.mnRequiredGLVersion = 2.0;

    return makeDissolveTransition(aLeavingSlide, aEnteringSlide, aSettings);
}

namespace
{

class VortexTransition : public ShaderTransition
{
public:
    VortexTransition(const TransitionScene& rScene, const TransitionSettings& rSettings, int nNX, int nNY)
        : ShaderTransition(rScene, rSettings)
        , mnTileInfoLocation(0)
        , mnTileInfoBuffer(0)
        , maNumTiles(nNX,nNY)
    {
        mvTileInfo.resize(6*maNumTiles.x*maNumTiles.y);
    }

private:
    virtual void prepare( double nTime, double SlideWidth, double SlideHeight, double DispWidth, double DispHeight ) override;

    virtual void finish( double nTime, double SlideWidth, double SlideHeight, double DispWidth, double DispHeight ) override;

    virtual GLuint makeShader() override;

    GLint mnTileInfoLocation;
    GLuint mnTileInfoBuffer;

    glm::ivec2 maNumTiles;

    std::vector<GLfloat> mvTileInfo;
};

void VortexTransition::prepare( double, double, double, double, double )
{
    glDisable(GL_CULL_FACE);

    glBindBuffer(GL_ARRAY_BUFFER, mnTileInfoBuffer);
    CHECK_GL_ERROR();
    glEnableVertexAttribArray(mnTileInfoLocation);
    CHECK_GL_ERROR();
    glVertexAttribPointer(mnTileInfoLocation, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
    CHECK_GL_ERROR();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    CHECK_GL_ERROR();
}

void VortexTransition::finish( double, double, double, double, double )
{
    glEnable(GL_CULL_FACE);
}

GLuint VortexTransition::makeShader()
{
    GLuint nProgram = OpenGLHelper::LoadShaders( "vortexVertexShader", "vortexFragmentShader" );

    if (nProgram)
    {
        mnTileInfoLocation = glGetAttribLocation(nProgram, "tileInfo");
        CHECK_GL_ERROR();

        glUseProgram(nProgram);
        CHECK_GL_ERROR();

        GLint nNumTilesLocation = glGetUniformLocation(nProgram, "numTiles");
        CHECK_GL_ERROR();

        glUniform2iv(nNumTilesLocation, 1, glm::value_ptr(maNumTiles));
        CHECK_GL_ERROR();
    }

    glGenBuffers(1, &mnTileInfoBuffer);
    CHECK_GL_ERROR();

    // We store the (x,y) indexes of the tile each vertex belongs to in a float, so they must fit.
    assert(maNumTiles.x < 256);
    assert(maNumTiles.y < 256);

    // Two triangles, i.e. six vertices, per tile
    {
        int n = 0;
        for (int x = 0; x < maNumTiles.x; x++)
        {
            for (int y = 0; y < maNumTiles.y; y++)
            {
                for (int v = 0; v < 6; v++)
                {
                    mvTileInfo[n] = x + (y << 8) + (v << 16);
                    n++;
                }
            }
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, mnTileInfoBuffer);
    CHECK_GL_ERROR();
    glBufferData(GL_ARRAY_BUFFER, mvTileInfo.size()*sizeof(GLfloat), mvTileInfo.data(), GL_STATIC_DRAW);
    CHECK_GL_ERROR();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    CHECK_GL_ERROR();

    return nProgram;
}

std::shared_ptr<OGLTransitionImpl>
makeVortexTransition(const Primitives_t& rLeavingSlidePrimitives,
                     const Primitives_t& rEnteringSlidePrimitives,
                     const TransitionSettings& rSettings,
                     int NX,
                     int NY)
{
    return std::make_shared<VortexTransition>(TransitionScene(rLeavingSlidePrimitives, rEnteringSlidePrimitives),
                                              rSettings,
                                              NX, NY);
}

}

std::shared_ptr<OGLTransitionImpl> makeVortex()
{
    const int NX = 40, NY = 40;
    Primitive Slide;

    for (int x = 0; x < NX; x++)
    {
        for (int y = 0; y < NY; y++)
        {
            Slide.pushTriangle (glm::vec2 (fdiv(x,NX),fdiv(y,NY)), glm::vec2 (fdiv(x+1,NX),fdiv(y,NY)), glm::vec2 (fdiv(x,NX),fdiv(y+1,NY)));
            Slide.pushTriangle (glm::vec2 (fdiv(x+1,NX),fdiv(y,NY)), glm::vec2 (fdiv(x,NX),fdiv(y+1,NY)), glm::vec2 (fdiv(x+1,NX),fdiv(y+1,NY)));
        }
    }
    Primitives_t aLeavingSlide;
    aLeavingSlide.push_back (Slide);
    Primitives_t aEnteringSlide;
    aEnteringSlide.push_back (Slide);

    TransitionSettings aSettings;
    aSettings.mbUseMipMapLeaving = aSettings.mbUseMipMapEntering = false;
    aSettings.mnRequiredGLVersion = 2.0;

    return makeVortexTransition(aLeavingSlide, aEnteringSlide, aSettings, NX, NY);
}

namespace
{

class RippleTransition : public ShaderTransition
{
public:
    RippleTransition(const TransitionScene& rScene, const TransitionSettings& rSettings, const glm::vec2& rCenter)
        : ShaderTransition(rScene, rSettings),
          maCenter(rCenter)
    {
    }

private:
    virtual GLuint makeShader() override;

    glm::vec2 maCenter;
};

GLuint RippleTransition::makeShader()
{
    GLuint nProgram = OpenGLHelper::LoadShaders( "basicVertexShader", "rippleFragmentShader" );

    if (nProgram)
    {
        glUseProgram(nProgram);
        CHECK_GL_ERROR();

        GLint nCenterLocation = glGetUniformLocation(nProgram, "center");
        CHECK_GL_ERROR();

        glUniform2fv(nCenterLocation, 1, glm::value_ptr(maCenter));
        CHECK_GL_ERROR();
    }

    return nProgram;
}

std::shared_ptr<OGLTransitionImpl>
makeRippleTransition(const Primitives_t& rLeavingSlidePrimitives,
                     const Primitives_t& rEnteringSlidePrimitives,
                     const TransitionSettings& rSettings)
{
    // The center point should be adjustable by the user, but we have no way to do that in the UI
    return std::make_shared<RippleTransition>(TransitionScene(rLeavingSlidePrimitives, rEnteringSlidePrimitives),
                                              rSettings,
                                              glm::vec2(0.5, 0.5));
}

}

std::shared_ptr<OGLTransitionImpl> makeRipple()
{
    Primitive Slide;

    Slide.pushTriangle (glm::vec2 (0,0), glm::vec2 (1,0), glm::vec2 (0,1));
    Slide.pushTriangle (glm::vec2 (1,0), glm::vec2 (0,1), glm::vec2 (1,1));

    Primitives_t aLeavingSlide;
    aLeavingSlide.push_back (Slide);

    Primitives_t aEnteringSlide;
    aEnteringSlide.push_back (Slide);

    TransitionSettings aSettings;
    aSettings.mbUseMipMapLeaving = aSettings.mbUseMipMapEntering = false;
    aSettings.mnRequiredGLVersion = 2.0;

    return makeRippleTransition(aLeavingSlide, aEnteringSlide, aSettings);
}

void createHexagon(Primitive& aHexagon, const int x, const int y, const int NX, const int NY)
{
    if (y % 4 == 0)
    {
        aHexagon.pushTriangle(vec(x-1, y-1, NX, NY), vec(x,   y-2, NX, NY), vec(x, y+0.5, NX, NY));
        aHexagon.pushTriangle(vec(x,   y-2, NX, NY), vec(x+1, y-1, NX, NY), vec(x, y+0.5, NX, NY));
        aHexagon.pushTriangle(vec(x+1, y-1, NX, NY), vec(x+1, y,   NX, NY), vec(x, y+0.5, NX, NY));
        aHexagon.pushTriangle(vec(x+1, y,   NX, NY), vec(x,   y+1, NX, NY), vec(x, y+0.5, NX, NY));
        aHexagon.pushTriangle(vec(x,   y+1, NX, NY), vec(x-1, y,   NX, NY), vec(x, y+0.5, NX, NY));
        aHexagon.pushTriangle(vec(x-1, y,   NX, NY), vec(x-1, y-1, NX, NY), vec(x, y+0.5, NX, NY));
    }
    else
    {
        aHexagon.pushTriangle(vec(x-2, y-1, NX, NY), vec(x-1, y-2, NX, NY), vec(x, y+0.5, NX, NY));
        aHexagon.pushTriangle(vec(x-1, y-2, NX, NY), vec(x,   y-1, NX, NY), vec(x, y+0.5, NX, NY));
        aHexagon.pushTriangle(vec(x,   y-1, NX, NY), vec(x,   y,   NX, NY), vec(x, y+0.5, NX, NY));
        aHexagon.pushTriangle(vec(x,   y,   NX, NY), vec(x-1, y+1, NX, NY), vec(x, y+0.5, NX, NY));
        aHexagon.pushTriangle(vec(x-1, y+1, NX, NY), vec(x-2, y,   NX, NY), vec(x, y+0.5, NX, NY));
        aHexagon.pushTriangle(vec(x-2, y,   NX, NY), vec(x-2, y-1, NX, NY), vec(x, y+0.5, NX, NY));
    }
}

std::shared_ptr<OGLTransitionImpl> makeGlitter()
{
    const int NX = 80;
    const int NY = NX * 4 / 3;

    Primitives_t aLeavingSlide;
    Primitives_t aEnteringSlide;

    for (int y = 0; y < NY+2; y+=2)
    {
        for (int x = 0; x < NX+2; x+=2)
        {
            Primitive aHexagon;
            createHexagon(aHexagon, x, y, NX, NY);

            glm::vec3 aCenter = aHexagon.getVertex(2);

            float fRandom = comphelper::rng::uniform_real_distribution(-0.25, std::nextafter(0.2, DBL_MAX));

            double fDelta = 0.6 + fRandom;
            double fHorizontal = fdiv(x, NX + 2) * fDelta;

            double fStart = fHorizontal;
            double fEnd   = fHorizontal + (1.0 - fDelta);

            aHexagon.Operations.push_back(makeSRotate(glm::vec3(0, 1, 0), aCenter, 180 , true, fStart, fEnd));
            aLeavingSlide.push_back(aHexagon);

            aHexagon.Operations.push_back(makeSRotate(glm::vec3(0, 1, 0), aCenter, 180 , false, fStart, fEnd));
            aEnteringSlide.push_back(aHexagon);
        }
    }

    return makeSimpleTransition(aLeavingSlide, aEnteringSlide);
}

std::shared_ptr<OGLTransitionImpl> makeHoneycomb()
{
    const int NX = 21;
    const int NY = 21;

    Primitives_t aLeavingSlide;
    Primitives_t aEnteringSlide;

    float fRandom = 0.0f;

    int centerX = NX / 2;
    int centerY = NY / 2;

    for (int y = 0; y < NY+2; y+=2)
    {
        for (int x = 0; x < NX+2; x+=2)
        {
            Primitive aHexagon;
            createHexagon(aHexagon, x, y, NX, NY);

            fRandom = comphelper::rng::uniform_real_distribution(0.2, std::nextafter(0.7, DBL_MAX));

            aHexagon.Operations.push_back(makeSRotate(glm::vec3(0, 0, 1), glm::vec3(0, 0, 0), 0 , false, -1, 0.1));
            aHexagon.Operations.push_back(makeSRotate(glm::vec3(0, 0, 1), glm::vec3(0, 0, 0), 90 , true, 0.2, 1.0));
            if (x <= 0 || y <= 0 || x >= NX || y >= NY)
            {
                aHexagon.Operations.push_back(makeSTranslate(glm::vec3 (0, 0, 100), false, 0.1, 1.0));
            }
            else if ((centerX - 1 <= x && x <= centerX + 1 && centerY - 1 <= y && y <= centerY + 1 ))
            {
                aHexagon.Operations.push_back(makeSTranslate(glm::vec3 (0, 0, 7), true, 0.0, 1.0));
                aHexagon.Operations.push_back(makeSTranslate(glm::vec3 (0, 0, 100), false, 0.1, 1.0));
            }
            else
            {
                aHexagon.Operations.push_back(makeSTranslate(glm::vec3 (0, 0, 7), true, 0.0, 1.0));
                aHexagon.Operations.push_back(makeSTranslate(glm::vec3 (0, 0, 100), false, fRandom, 1.0));
            }

            aLeavingSlide.push_back(aHexagon);

            aHexagon.Operations.clear();

            aHexagon.Operations.push_back(makeSRotate(glm::vec3(0, 0, 1), glm::vec3(0, 0, 0), -90 , false, -1, 0.0));
            aHexagon.Operations.push_back(makeSRotate(glm::vec3(0, 0, 1), glm::vec3(0, 0, 0), 90 , true, 0.0, 0.8));

            aHexagon.Operations.push_back(makeSTranslate(glm::vec3(-100, 0, -20), false, -1, 0));

            if (x <= 0 || y <= 0 || x >= NX || y >= NY)
            {
                fRandom = comphelper::rng::uniform_real_distribution(0.85, std::nextafter(0.95, DBL_MAX));
                aHexagon.Operations.push_back(makeSTranslate(glm::vec3(100, 0, 0), false, fRandom, 0.95));
            }
            else
            {
                fRandom = comphelper::rng::uniform_real_distribution(0.3, std::nextafter(0.8, DBL_MAX));
                aHexagon.Operations.push_back(makeSTranslate(glm::vec3(100, 0, 0), false, fRandom, 0.9));
            }
            aHexagon.Operations.push_back(makeSTranslate(glm::vec3 (0, 0, 20), true, 0.1, 1.0));
            aEnteringSlide.push_back(aHexagon);
        }
    }

    return makeSimpleTransition(aLeavingSlide, aEnteringSlide);
}

std::shared_ptr<OGLTransitionImpl> makeNewsflash()
{
    Primitive Slide;

    Slide.pushTriangle(glm::vec2(0,0),glm::vec2(1,0),glm::vec2(0,1));
    Slide.pushTriangle(glm::vec2(1,0),glm::vec2(0,1),glm::vec2(1,1));
    Slide.Operations.push_back(makeSRotate(glm::vec3(0,0,1),glm::vec3(0,0,0),3000,true,0,0.5));
    Slide.Operations.push_back(makeSScale(glm::vec3(0.01,0.01,0.01),glm::vec3(0,0,0),true,0,0.5));
    Slide.Operations.push_back(makeSTranslate(glm::vec3(-10000, 0, 0),false, 0.5, 2));
    Primitives_t aLeavingSlide;
    aLeavingSlide.push_back(Slide);

    Slide.Operations.clear();
    Slide.Operations.push_back(makeSRotate(glm::vec3(0,0,1),glm::vec3(0,0,0),-3000,true,0.5,1));
    Slide.Operations.push_back(makeSTranslate(glm::vec3(-100, 0, 0),false, -1, 1));
    Slide.Operations.push_back(makeSTranslate(glm::vec3(100, 0, 0),false, 0.5, 1));
    Slide.Operations.push_back(makeSScale(glm::vec3(0.01,0.01,0.01),glm::vec3(0,0,0),false,-1,1));
    Slide.Operations.push_back(makeSScale(glm::vec3(100,100,100),glm::vec3(0,0,0),true,0.5,1));
    Primitives_t aEnteringSlide;
    aEnteringSlide.push_back(Slide);

    Operations_t aOverallOperations;
    aOverallOperations.push_back(makeSRotate(glm::vec3(0,0,1),glm::vec3(0.2,0.2,0),1080,true,0,1));

    return makeSimpleTransition(aLeavingSlide, aEnteringSlide, aOverallOperations);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
