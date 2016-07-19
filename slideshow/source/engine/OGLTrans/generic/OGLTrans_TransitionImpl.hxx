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
#include <glm/gtc/type_ptr.hpp>

#include <GL/glew.h>

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
        mnRequiredGLVersion( 2.1f )
    {
    }

    /** Whether to use mipmaping for slides textures
     */
    bool mbUseMipMapLeaving;
    bool mbUseMipMapEntering;

    /** which GL version does the transition require
     */
    float mnRequiredGLVersion;
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
class OGLTransitionImpl
{
public:
    virtual ~OGLTransitionImpl();

    OGLTransitionImpl(const OGLTransitionImpl&) = delete;
    OGLTransitionImpl& operator=(const OGLTransitionImpl&) = delete;

    /** Prepare transition.
      */
    bool prepare( sal_Int32 glLeavingSlideTex, sal_Int32 glEnteringSlideTex );
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

    TransitionScene const& getScene() const
    {
        return maScene;
    }

    void displaySlide( double nTime, sal_Int32 glSlideTex, const Primitives_t& primitives, double SlideWidthScale, double SlideHeightScale );
    void displayUnbufferedSlide( double nTime, sal_Int32 glSlideTex, const Primitives_t& primitives, double SlideWidthScale, double SlideHeightScale );
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

    /** This function is called in prepare method to create the GL program.
      *
      * It is a pure virtual to make sure no class will use a default one.
      */
    virtual GLuint makeShader() const = 0;

private:
    const TransitionScene maScene;
    const TransitionSettings maSettings;

    /** Calculates the projection and model/view matrices, and upload them.
     */
    void uploadModelViewProjectionMatrices();

    /** Uniform locations for transform matrices
     */
    GLint m_nPrimitiveTransformLocation = -1;
    GLint m_nSceneTransformLocation = -1;
    GLint m_nOperationsTransformLocation = -1;

    /** Per-vertex attribute locations
     */
    GLint m_nPositionLocation = -1;
    GLint m_nNormalLocation = -1;
    GLint m_nTexCoordLocation = -1;

    GLuint m_nVertexArrayObject = 0u;

    std::vector<int> m_nFirstIndices;

protected:
    /** GLSL program object
     */
    GLuint m_nProgramObject = 0u;

    /** VBO in which to put primitive data
     */
    GLuint m_nVertexBufferObject = 0u;

    /** Location of the "time" uniform
     */
    GLint m_nTimeLocation = -1;
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

class SceneObject
{
public:
    SceneObject();
    virtual ~SceneObject();
    SceneObject(const SceneObject&) = delete;
    SceneObject& operator=(const SceneObject&) = delete;

    virtual void prepare(GLuint /* program */) {}
    virtual void display(GLint sceneTransformLocation, GLint primitiveTransformLocation, double nTime, double SlideWidth, double SlideHeight, double DispWidth, double DispHeight ) const;
    virtual void finish() {}

    void pushPrimitive (const Primitive &p);

protected:
    /** All the surrounding scene primitives
    */
    Primitives_t maPrimitives;
    std::vector<int> maFirstIndices;
};

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoord;
};
static_assert(sizeof(Vertex) == (3 + 3 + 2) * 4, "Vertex struct has wrong size/alignment");

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

    void applyOperations(glm::mat4& matrix, double nTime, double SlideWidthScale, double SlideHeightScale) const;
    void display(GLint primitiveTransformLocation, double nTime, double WidthScale, double HeightScale) const;
    void display(GLint primitiveTransformLocation, double nTime, double WidthScale, double HeightScale, int first) const;

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
    const glm::vec3& getVertex(int n) const {return Vertices[n].position;}

    /** accessor for the size of the vertices data

        @return
        the size in bytes of the Vertices data
    */
    int getVerticesSize() const {return Vertices.size() * sizeof(Vertex);}

    /** copies all vertices to the C array passed

        @return
        the number of written vertices
    */
    int writeVertices(Vertex *location) const {
        std::copy(Vertices.begin(), Vertices.end(), location);
        return Vertices.size();
    }

    /** list of Operations to be performed on this primitive.These operations will be called in the order they were pushed back in. In OpenGL this effectively uses the operations in the opposite order they were pushed back.

        @return
        the list of Operations

    */
    Operations_t Operations;

private:
    /** list of vertices
    */
    std::vector<Vertex> Vertices;
};

#endif // INCLUDED_SLIDESHOW_TRANSITION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
