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
#include <vcl/opengl/OpenGLHelper.hxx>

#include <utility>

#include <boost/make_shared.hpp>
#include <comphelper/random.hxx>

#include "OGLTrans_TransitionImpl.hxx"
#include <math.h>

using boost::make_shared;
using boost::shared_ptr;

using std::max;
using std::min;
using std::vector;

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

void OGLTransitionImpl::prepare( ::sal_Int32 glLeavingSlideTex, ::sal_Int32 glEnteringSlideTex )
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

static void blendSlide()
{

    float showHeight = 0.4;
    GLuint m_vertexBuffer;

    glDisable( GL_DEPTH_TEST );

    GLfloat vertices[] = { -1, -1, 0,
                           -1, showHeight, 0,
                           1, showHeight, 0,
                           1, -1, 0 };

    GLfloat color[] = { 0, 0, 0, 0.25,
                        0, 0, 0, 1,
                        0, 0, 0, 1,
                        0, 0, 0, 0.25 };

    glGenBuffers(1, &m_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glColorPointer(4, GL_FLOAT, 0, color);
    glDrawArrays(GL_TRIANGLE_FAN,0,4);

    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    GLfloat vertices2[] = { -1, showHeight, 0,
                            -1,  1, 0,
                            1,  1, 0,
                            1, showHeight, 0 };

    GLfloat color2[] = { 0, 0, 0, 1,
                         0, 0, 0, 1,
                         0, 0, 0, 1,
                         0, 0, 0, 1 };

    glGenBuffers(1, &m_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, vertices2);
    glColorPointer(4, GL_FLOAT, 0, color2);
    glDrawArrays(GL_TRIANGLE_FAN,0,4);

    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    glEnable( GL_DEPTH_TEST );

}

static void slideShadow( double nTime, const Primitive& primitive, double sw, double sh )
{


    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_LIGHTING);

    glPushMatrix();
    primitive.applyOperations( nTime, sw, sh );
    blendSlide();
    glPopMatrix();

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);

}

void OGLTransitionImpl::prepare( double, double, double, double, double )
{
}

void OGLTransitionImpl::prepareTransition( ::sal_Int32, ::sal_Int32 )
{
}

void OGLTransitionImpl::finishTransition()
{
}

void OGLTransitionImpl::displaySlides_( double nTime, ::sal_Int32 glLeavingSlideTex, ::sal_Int32 glEnteringSlideTex, double SlideWidthScale, double SlideHeightScale )
{

    applyOverallOperations( nTime, SlideWidthScale, SlideHeightScale );

    glEnable(GL_TEXTURE_2D);
    displaySlide( nTime, glLeavingSlideTex, maScene.getLeavingSlide(), SlideWidthScale, SlideHeightScale );
    displaySlide( nTime, glEnteringSlideTex, maScene.getEnteringSlide(), SlideWidthScale, SlideHeightScale );
}

void OGLTransitionImpl::display( double nTime, ::sal_Int32 glLeavingSlideTex, ::sal_Int32 glEnteringSlideTex,
                                 double SlideWidth, double SlideHeight, double DispWidth, double DispHeight )
{
    const double SlideWidthScale = SlideWidth/DispWidth;
    const double SlideHeightScale = SlideHeight/DispHeight;


    prepare( nTime, SlideWidth, SlideHeight, DispWidth, DispHeight );


    glPushMatrix();

    displaySlides_( nTime, glLeavingSlideTex, glEnteringSlideTex, SlideWidthScale, SlideHeightScale );

    displayScene( nTime, SlideWidth, SlideHeight, DispWidth, DispHeight );

    glPopMatrix();

}

void OGLTransitionImpl::applyOverallOperations( double nTime, double SlideWidthScale, double SlideHeightScale )
{
    const Operations_t& rOverallOperations(maScene.getOperations());
    for(size_t i(0); i != rOverallOperations.size(); ++i)
        rOverallOperations[i]->interpolate(nTime,SlideWidthScale,SlideHeightScale);
}

void
        OGLTransitionImpl::displaySlide(
        const double nTime,
        const ::sal_Int32 glSlideTex, const Primitives_t& primitives,
        double SlideWidthScale, double SlideHeightScale )
{

    //TODO change to foreach
    glBindTexture(GL_TEXTURE_2D, glSlideTex);


    // display slide reflection
    // note that depth test is turned off while blending the shadow
    // so the slides has to be rendered in right order, see rochade as example
    if( maSettings.mbReflectSlides ) {
        double surfaceLevel = -0.04;

        /* reflected slides */
        glPushMatrix();

        glScaled( 1, -1, 1 );
        glTranslated( 0, 2 - surfaceLevel, 0 );

        glCullFace(GL_FRONT);
        for(size_t i(0); i < primitives.size(); ++i)
            primitives[i].display(nTime, SlideWidthScale, SlideHeightScale);
        glCullFace(GL_BACK);

        //slideShadow( nTime, primitives[0], SlideWidthScale, SlideHeightScale );

        glPopMatrix();
    }

    for(size_t i(0); i < primitives.size(); ++i)
        primitives[i].display(nTime, SlideWidthScale, SlideHeightScale);

}

void OGLTransitionImpl::displayScene( double nTime, double SlideWidth, double SlideHeight, double DispWidth, double DispHeight )
{

    const SceneObjects_t& rSceneObjects(maScene.getSceneObjects());
    glEnable(GL_TEXTURE_2D);
    for(size_t i(0); i != rSceneObjects.size(); ++i)
        rSceneObjects[i]->display(nTime, SlideWidth, SlideHeight, DispWidth, DispHeight);

}

void Primitive::display(double nTime, double WidthScale, double HeightScale) const
{

    glPushMatrix();


    applyOperations( nTime, WidthScale, HeightScale );


    glEnableClientState( GL_VERTEX_ARRAY );

    if(!Normals.empty())
    {

        glNormalPointer( GL_FLOAT , 0 , &Normals[0] );

        glEnableClientState( GL_NORMAL_ARRAY );

    }

    glEnableClientState( GL_TEXTURE_COORD_ARRAY );

    glTexCoordPointer( 2, GL_FLOAT, 0, &TexCoords[0] );

    glVertexPointer( 3, GL_FLOAT, 0, &Vertices[0] );

    glDrawArrays( GL_TRIANGLES, 0, Vertices.size() );

    glPopMatrix();

}

void Primitive::applyOperations(double nTime, double WidthScale, double HeightScale) const
{

    for(size_t i(0); i < Operations.size(); ++i)
        Operations[i]->interpolate( nTime ,WidthScale,HeightScale);
    glScaled(WidthScale,HeightScale,1);

}

void SceneObject::display(double nTime, double /* SlideWidth */, double /* SlideHeight */, double DispWidth, double DispHeight ) const
{

    for(size_t i(0); i < maPrimitives.size(); ++i) {
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

shared_ptr<OGLTransitionImpl>
makeSimpleTransition()
{
    return make_shared<SimpleTransition>();
}

shared_ptr<OGLTransitionImpl>
makeSimpleTransition(
        const Primitives_t& rLeavingSlidePrimitives,
        const Primitives_t& rEnteringSlidePrimitives,
        const Operations_t& rOverallOperations,
        const SceneObjects_t& rSceneObjects,
        const TransitionSettings& rSettings = TransitionSettings())
{
    return make_shared<SimpleTransition>(
            TransitionScene(rLeavingSlidePrimitives, rEnteringSlidePrimitives, rOverallOperations, rSceneObjects),
            rSettings)
        ;
}

shared_ptr<OGLTransitionImpl>
makeSimpleTransition(
        const Primitives_t& rLeavingSlidePrimitives,
        const Primitives_t& rEnteringSlidePrimitives,
        const Operations_t& rOverallOperations,
        const TransitionSettings& rSettings = TransitionSettings())
{
    return makeSimpleTransition(rLeavingSlidePrimitives, rEnteringSlidePrimitives, rOverallOperations, SceneObjects_t(), rSettings);
}

shared_ptr<OGLTransitionImpl>
makeSimpleTransition(
        const Primitives_t& rLeavingSlidePrimitives,
        const Primitives_t& rEnteringSlidePrimitives,
        const SceneObjects_t& rSceneObjects,
        const TransitionSettings& rSettings = TransitionSettings())
{
    return makeSimpleTransition(rLeavingSlidePrimitives, rEnteringSlidePrimitives, Operations_t(), rSceneObjects, rSettings);
}

shared_ptr<OGLTransitionImpl>
makeSimpleTransition(
        const Primitives_t& rLeavingSlidePrimitives,
        const Primitives_t& rEnteringSlidePrimitives,
        const TransitionSettings& rSettings = TransitionSettings())
{
    return makeSimpleTransition(rLeavingSlidePrimitives, rEnteringSlidePrimitives, Operations_t(), SceneObjects_t(), rSettings);
}

}

boost::shared_ptr<OGLTransitionImpl> makeOutsideCubeFaceToLeft()
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

boost::shared_ptr<OGLTransitionImpl> makeInsideCubeFaceToLeft()
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

boost::shared_ptr<OGLTransitionImpl> makeFallLeaving()
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

boost::shared_ptr<OGLTransitionImpl> makeTurnAround()
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

boost::shared_ptr<OGLTransitionImpl> makeTurnDown()
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

boost::shared_ptr<OGLTransitionImpl> makeIris()
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

    shared_ptr<Iris> pIris = make_shared<Iris>();
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
    virtual void displaySlides_( double nTime, ::sal_Int32 glLeavingSlideTex, ::sal_Int32 glEnteringSlideTex, double SlideWidthScale, double SlideHeightScale ) SAL_OVERRIDE;
};

void RochadeTransition::displaySlides_( double nTime, ::sal_Int32 glLeavingSlideTex, ::sal_Int32 glEnteringSlideTex, double SlideWidthScale, double SlideHeightScale )
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

shared_ptr<OGLTransitionImpl>
makeRochadeTransition(
        const Primitives_t& rLeavingSlidePrimitives,
        const Primitives_t& rEnteringSlidePrimitives,
        const TransitionSettings& rSettings)
{
    return make_shared<RochadeTransition>(
            TransitionScene(rLeavingSlidePrimitives, rEnteringSlidePrimitives),
            rSettings)
        ;

}
}

boost::shared_ptr<OGLTransitionImpl> makeRochade()
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

boost::shared_ptr<OGLTransitionImpl> makeRevolvingCircles( ::sal_uInt16 nCircles , ::sal_uInt16 nPointsOnCircles )
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
    vector<glm::vec2> unScaledTexCoords;
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

boost::shared_ptr<OGLTransitionImpl> makeHelix( ::sal_uInt16 nRows )
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

        Tile.Operations.push_back( makeSRotate( glm::vec3( 0 , 1 , 0 ) , ( Tile.getVertices()[1] + Tile.getVertices()[3] )/2.0f , 180 ,
                                                true,min(max(static_cast<double>(i - nRows/2.0)*invN/2.0,0.0),1.0),
                                                min(max(static_cast<double>(i + nRows/2.0)*invN/2.0,0.0),1.0) ) );

        aLeavingSlide.push_back(Tile);

        Tile.Operations.push_back( makeSRotate( glm::vec3( 0 , 1 , 0 ) , ( Tile.getVertices()[1] + Tile.getVertices()[3] )/2.0f , -180 , false,0.0,1.0) );

        aEnteringSlide.push_back(Tile);

        iDn += invN;
        iPDn += invN;
    }

    return makeSimpleTransition(aLeavingSlide, aEnteringSlide);
}

boost::shared_ptr<OGLTransitionImpl> makeNByMTileFlip( ::sal_uInt16 n, ::sal_uInt16 m )
{
    double invN(1.0/static_cast<double>(n));
    double invM(1.0/static_cast<double>(m));
    double iDn = 0.0;
    double iPDn = invN;
    Primitives_t aLeavingSlide;
    Primitives_t aEnteringSlide;
    for(unsigned int i(0); i < n; ++i)
    {
        double jDm = 0.0;
        double jPDm = invM;
        for(unsigned int j(0); j < m; ++j)
        {
            Primitive Tile;

            Tile.pushTriangle(glm::vec2( iPDn , jDm ) , glm::vec2( iDn , jDm ) , glm::vec2( iDn , jPDm ));

            Tile.pushTriangle(glm::vec2( iPDn , jPDm ) , glm::vec2( iPDn , jDm ) , glm::vec2( iDn , jPDm ));//bottom left corner of tile

            Tile.Operations.push_back( makeSRotate( glm::vec3( 1 , 1 , 0 ) , ( Tile.getVertices()[1] + Tile.getVertices()[3] )/2.0f , 180 , true, iDn*jDm/2.0 , ((iPDn*jPDm)+1.0)/2.0 ) );
            aLeavingSlide.push_back(Tile);
            Tile.Operations.push_back( makeSRotate( glm::vec3( 1 , 1 , 0 ) , ( Tile.getVertices()[1] + Tile.getVertices()[3] )/2.0f , -180, false, iDn*jDm/2.0 , ((iPDn*jPDm)+1.0)/2.0 ) );

            aEnteringSlide.push_back(Tile);

            jDm += invM;
            jPDm += invM;
        }
        iDn += invN;
        iPDn += invN;
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

boost::shared_ptr<SRotate>
makeSRotate(const glm::vec3& Axis,const glm::vec3& Origin,double Angle,bool bInter, double T0, double T1)
{
    return make_shared<SRotate>(Axis, Origin, Angle, bInter, T0, T1);
}

boost::shared_ptr<SScale>
makeSScale(const glm::vec3& Scale, const glm::vec3& Origin,bool bInter, double T0, double T1)
{
    return make_shared<SScale>(Scale, Origin, bInter, T0, T1);
}

boost::shared_ptr<STranslate>
makeSTranslate(const glm::vec3& Vector,bool bInter, double T0, double T1)
{
    return make_shared<STranslate>(Vector, bInter, T0, T1);
}

boost::shared_ptr<SEllipseTranslate>
makeSEllipseTranslate(double dWidth, double dHeight, double dStartPosition, double dEndPosition, bool bInter, double T0, double T1)
{
    return make_shared<SEllipseTranslate>(dWidth, dHeight, dStartPosition, dEndPosition, bInter, T0, T1);
}

boost::shared_ptr<RotateAndScaleDepthByWidth>
makeRotateAndScaleDepthByWidth(const glm::vec3& Axis,const glm::vec3& Origin,double Angle,bool bInter, double T0, double T1)
{
    return make_shared<RotateAndScaleDepthByWidth>(Axis, Origin, Angle, bInter, T0, T1);
}

boost::shared_ptr<RotateAndScaleDepthByHeight>
makeRotateAndScaleDepthByHeight(const glm::vec3& Axis,const glm::vec3& Origin,double Angle,bool bInter, double T0, double T1)
{
    return make_shared<RotateAndScaleDepthByHeight>(Axis, Origin, Angle, bInter, T0, T1);
}

inline double intervalInter(double t, double T0, double T1)
{
    return ( t - T0 ) / ( T1 - T0 );
}

void STranslate::interpolate(double t,double SlideWidthScale,double SlideHeightScale) const
{

    if(t <= mnT0)
        return;
    if(!mbInterpolate || t > mnT1)
        t = mnT1;
    t = intervalInter(t,mnT0,mnT1);
    glTranslated(SlideWidthScale*t*vector.x,SlideHeightScale*t*vector.y,t*vector.z);

}

void SRotate::interpolate(double t,double SlideWidthScale,double SlideHeightScale) const
{

    if(t <= mnT0)
        return;
    if(!mbInterpolate || t > mnT1)
        t = mnT1;
    t = intervalInter(t,mnT0,mnT1);
    glTranslated(SlideWidthScale*origin.x,SlideHeightScale*origin.y,origin.z);
    glScaled(SlideWidthScale,SlideHeightScale,1);
    glRotated(t*angle,axis.x,axis.y,axis.z);
    glScaled(1/SlideWidthScale,1/SlideHeightScale,1);
    glTranslated(-SlideWidthScale*origin.x,-SlideHeightScale*origin.y,-origin.z);

}

void SScale::interpolate(double t,double SlideWidthScale,double SlideHeightScale) const
{

    if(t <= mnT0)
        return;
    if(!mbInterpolate || t > mnT1)
        t = mnT1;
    t = intervalInter(t,mnT0,mnT1);
    glTranslated(SlideWidthScale*origin.x,SlideHeightScale*origin.y,origin.z);
    glScaled((1-t) + t*scale.x,(1-t) + t*scale.y,(1-t) + t*scale.z);
    glTranslated(-SlideWidthScale*origin.x,-SlideHeightScale*origin.y,-origin.z);

}

void RotateAndScaleDepthByWidth::interpolate(double t,double SlideWidthScale,double SlideHeightScale) const
{

    if(t <= mnT0)
        return;
    if(!mbInterpolate || t > mnT1)
        t = mnT1;
    t = intervalInter(t,mnT0,mnT1);
    glTranslated(SlideWidthScale*origin.x,SlideHeightScale*origin.y,SlideWidthScale*origin.z);
    glRotated(t*angle,axis.x,axis.y,axis.z);
    glTranslated(-SlideWidthScale*origin.x,-SlideHeightScale*origin.y,-SlideWidthScale*origin.z);

}

void RotateAndScaleDepthByHeight::interpolate(double t,double SlideWidthScale,double SlideHeightScale) const
{

    if(t <= mnT0)
        return;
    if(!mbInterpolate || t > mnT1)
        t = mnT1;
    t = intervalInter(t,mnT0,mnT1);
    glTranslated(SlideWidthScale*origin.x,SlideHeightScale*origin.y,SlideHeightScale*origin.z);
    glRotated(t*angle,axis.x,axis.y,axis.z);
    glTranslated(-SlideWidthScale*origin.x,-SlideHeightScale*origin.y,-SlideHeightScale*origin.z);

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

void SEllipseTranslate::interpolate(double t,double /* SlideWidthScale */,double /* SlideHeightScale */) const
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

    glTranslated(x, 0, y);
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
    , Normals(rvalue.Normals)
    , TexCoords(rvalue.TexCoords)
{
}

void Primitive::swap(Primitive& rOther)
{
    using std::swap;

    swap(Operations, rOther.Operations);
    swap(Vertices, rOther.Vertices);
    swap(Normals, rOther.Normals);
    swap(TexCoords, rOther.TexCoords);
}

void Primitive::pushTriangle(const glm::vec2& SlideLocation0,const glm::vec2& SlideLocation1,const glm::vec2& SlideLocation2)
{
    vector<glm::vec3> Verts;
    vector<glm::vec2> Texs;
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

    Vertices.push_back(Verts[0]);
    Vertices.push_back(Verts[1]);
    Vertices.push_back(Verts[2]);

    TexCoords.push_back(Texs[0]);
    TexCoords.push_back(Texs[1]);
    TexCoords.push_back(Texs[2]);

    Normals.push_back(glm::vec3(0,0,1));//all normals always face the screen when untransformed.
    Normals.push_back(glm::vec3(0,0,1));//all normals always face the screen when untransformed.
    Normals.push_back(glm::vec3(0,0,1));//all normals always face the screen when untransformed.
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
    virtual void prepare( double nTime, double SlideWidth, double SlideHeight, double DispWidth, double DispHeight ) SAL_OVERRIDE;
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

shared_ptr<OGLTransitionImpl>
makeDiamondTransition(const TransitionSettings& rSettings)
{
    return make_shared<DiamondTransition>(TransitionScene(), rSettings);
}

}

boost::shared_ptr<OGLTransitionImpl> makeDiamond()
{
    TransitionSettings aSettings;
    aSettings.mbUseMipMapLeaving = aSettings.mbUseMipMapEntering = false;

    return makeDiamondTransition(aSettings);
}

boost::shared_ptr<OGLTransitionImpl> makeVenetianBlinds( bool vertical, int parts )
{
    static double t30 = tan( M_PI/6.0 );
    double n, ln = 0;
    double p = 1.0/parts;

    Primitives_t aLeavingSlide;
    Primitives_t aEnteringSlide;
    for( int i=0; i<parts; i++ ) {
        Primitive Slide;
        n = (i + 1)/(double)parts;
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
    virtual void displaySlides_( double nTime, ::sal_Int32 glLeavingSlideTex, ::sal_Int32 glEnteringSlideTex, double SlideWidthScale, double SlideHeightScale ) SAL_OVERRIDE;
};

void FadeSmoothlyTransition::displaySlides_( double nTime, ::sal_Int32 glLeavingSlideTex, ::sal_Int32 glEnteringSlideTex, double SlideWidthScale, double SlideHeightScale )
{

    applyOverallOperations( nTime, SlideWidthScale, SlideHeightScale );


    glDisable(GL_DEPTH_TEST);


    displaySlide( nTime, glLeavingSlideTex, getScene().getLeavingSlide(), SlideWidthScale, SlideHeightScale );



    glDisable(GL_LIGHTING);

    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glColor4f( 1, 1, 1, nTime );

    displaySlide( nTime, glEnteringSlideTex, getScene().getEnteringSlide(), SlideWidthScale, SlideHeightScale );

    glDisable(GL_BLEND);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glEnable(GL_LIGHTING);


    glEnable(GL_DEPTH_TEST);

}

shared_ptr<OGLTransitionImpl>
makeFadeSmoothlyTransition(
        const Primitives_t& rLeavingSlidePrimitives,
        const Primitives_t& rEnteringSlidePrimitives,
        const TransitionSettings& rSettings)
{
    return make_shared<FadeSmoothlyTransition>(
            TransitionScene(rLeavingSlidePrimitives, rEnteringSlidePrimitives),
            rSettings)
        ;
}

}

boost::shared_ptr<OGLTransitionImpl> makeFadeSmoothly()
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
    virtual void displaySlides_( double nTime, ::sal_Int32 glLeavingSlideTex, ::sal_Int32 glEnteringSlideTex, double SlideWidthScale, double SlideHeightScale ) SAL_OVERRIDE;
};

void FadeThroughBlackTransition::displaySlides_( double nTime, ::sal_Int32 glLeavingSlideTex, ::sal_Int32 glEnteringSlideTex, double SlideWidthScale, double SlideHeightScale )
{

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

}

shared_ptr<OGLTransitionImpl>
makeFadeThroughBlackTransition(
        const Primitives_t& rLeavingSlidePrimitives,
        const Primitives_t& rEnteringSlidePrimitives,
        const TransitionSettings& rSettings)
{
    return make_shared<FadeThroughBlackTransition>(
            TransitionScene(rLeavingSlidePrimitives, rEnteringSlidePrimitives),
            rSettings)
        ;
}

}

boost::shared_ptr<OGLTransitionImpl> makeFadeThroughBlack()
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
    virtual void displaySlides_( double nTime, ::sal_Int32 glLeavingSlideTex, ::sal_Int32 glEnteringSlideTex, double SlideWidthScale, double SlideHeightScale ) SAL_OVERRIDE;
    virtual void prepareTransition( ::sal_Int32 glLeavingSlideTex, ::sal_Int32 glEnteringSlideTex ) SAL_OVERRIDE;
    virtual void finishTransition() SAL_OVERRIDE;
    virtual GLuint makeShader() = 0;

    void impl_preparePermShader();

private:
    /** GLSL program object
     */
    GLuint m_nProgramObject;

    /** various data */
    GLuint m_nHelperTexture;
};

void ShaderTransition::displaySlides_( double nTime, ::sal_Int32 glLeavingSlideTex, ::sal_Int32 glEnteringSlideTex,
                                              double SlideWidthScale, double SlideHeightScale )
{

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

}

void ShaderTransition::prepareTransition( ::sal_Int32 /* glLeavingSlideTex */, ::sal_Int32 /* glEnteringSlideTex */ )
{
    m_nProgramObject = makeShader();

    impl_preparePermShader();
}

void ShaderTransition::finishTransition()
{

    if( m_nProgramObject ) {
        glDeleteProgram( m_nProgramObject );
        m_nProgramObject = 0;
    }
    if ( m_nHelperTexture )
    {
        glDeleteTextures( 1, &m_nHelperTexture );
        m_nHelperTexture = 0;
    }

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

void ShaderTransition::impl_preparePermShader()
{

    if( m_nProgramObject ) {
        glUseProgram( m_nProgramObject );

        GLint location = glGetUniformLocation( m_nProgramObject, "leavingSlideTexture" );
        if( location != -1 ) {
            glUniform1i( location, 0 );  // texture unit 0
        }

        glActiveTexture(GL_TEXTURE1);
        if( !m_nHelperTexture )
            initPermTexture( &m_nHelperTexture );
        glActiveTexture(GL_TEXTURE0);

        location = glGetUniformLocation( m_nProgramObject, "permTexture" );
        if( location != -1 ) {
            glUniform1i( location, 1 );  // texture unit 1
        }

        location = glGetUniformLocation( m_nProgramObject, "enteringSlideTexture" );
        if( location != -1 ) {
            glUniform1i( location, 2 );  // texture unit 2
        }
    }

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
    virtual GLuint makeShader() SAL_OVERRIDE;
};

GLuint StaticNoiseTransition::makeShader()
{
    return OpenGLHelper::LoadShaders( "basicVertexShader", "staticFragmentShader" );
}

shared_ptr<OGLTransitionImpl>
makeStaticNoiseTransition(
        const Primitives_t& rLeavingSlidePrimitives,
        const Primitives_t& rEnteringSlidePrimitives,
        const TransitionSettings& rSettings)
{
    return make_shared<StaticNoiseTransition>(
            TransitionScene(rLeavingSlidePrimitives, rEnteringSlidePrimitives),
            rSettings)
        ;
}

}

boost::shared_ptr<OGLTransitionImpl> makeStatic()
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
    virtual GLuint makeShader() SAL_OVERRIDE;
};

GLuint DissolveTransition::makeShader()
{
    return OpenGLHelper::LoadShaders( "basicVertexShader", "dissolveFragmentShader" );
}

shared_ptr<OGLTransitionImpl>
makeDissolveTransition(
        const Primitives_t& rLeavingSlidePrimitives,
        const Primitives_t& rEnteringSlidePrimitives,
        const TransitionSettings& rSettings)
{
    return make_shared<DissolveTransition>(
            TransitionScene(rLeavingSlidePrimitives, rEnteringSlidePrimitives),
            rSettings)
        ;
}

}

boost::shared_ptr<OGLTransitionImpl> makeDissolve()
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

boost::shared_ptr<OGLTransitionImpl> makeNewsflash()
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
