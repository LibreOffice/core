/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ogl_canvastools.hxx"

#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/tools/tools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygontriangulator.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <vector>
#include <com/sun/star/rendering/ARGBColor.hpp>

#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

//subdivision count of bezier segments
#define COUNT_OF_ADAPTIVE_SUBDIVISION 40

using namespace ::com::sun::star;

namespace oglcanvas
{
    // triangulates polygon before
    void renderComplexPolyPolygon( const ::basegfx::B2DPolyPolygon& rPolyPoly, RenderHelper *renderHelper,
        glm::vec4 color)
    {
        ::basegfx::B2DPolyPolygon aPolyPoly(rPolyPoly);
        if( aPolyPoly.areControlPointsUsed() )
            aPolyPoly = rPolyPoly.getAdaptiveSubdivision(COUNT_OF_ADAPTIVE_SUBDIVISION);
        const ::basegfx::B2DPolygon& rTriangulatedPolygon(
            ::basegfx::triangulator::triangulate(aPolyPoly));
        if(rTriangulatedPolygon.count()>0)
        {
            std::vector<glm::vec2> vertices;
            vertices.reserve(rTriangulatedPolygon.count());
            for( sal_uInt32 i=0; i<rTriangulatedPolygon.count(); i++ )
            {
                const ::basegfx::B2DPoint& rPt( rTriangulatedPolygon.getB2DPoint(i) );
                vertices.push_back(glm::vec2(rPt.getX(),rPt.getY()));
            }

            renderHelper->renderVertexConstColor(vertices, color, GL_TRIANGLES);
        }
    }

    void renderTransformComplexPolygon( const ::basegfx::B2DPolyPolygon& rPolyPoly, RenderHelper *renderHelper,
        glm::vec4 color, glm::mat4 transform)
    {
        ::basegfx::B2DPolyPolygon aPolyPoly(rPolyPoly);
        if( aPolyPoly.areControlPointsUsed() )
            aPolyPoly = rPolyPoly.getAdaptiveSubdivision(COUNT_OF_ADAPTIVE_SUBDIVISION);
        const ::basegfx::B2DRange& rBounds(aPolyPoly.getB2DRange());
        const double nWidth=rBounds.getWidth();
        const double nHeight=rBounds.getHeight();
        const ::basegfx::B2DPolygon& rTriangulatedPolygon(
            ::basegfx::triangulator::triangulate(aPolyPoly));
        if(rTriangulatedPolygon.count()>0)
        {
            std::vector<glm::vec2> vertices;
            vertices.reserve(rTriangulatedPolygon.count());
            for( sal_uInt32 i=0; i<rTriangulatedPolygon.count(); i++ )
            {
                const ::basegfx::B2DPoint& rPt( rTriangulatedPolygon.getB2DPoint(i) );
                vertices.push_back(glm::vec2(rPt.getX(),rPt.getY()));
            }
            renderHelper->renderTextureTransform( vertices, nWidth, nHeight,  color, GL_TRIANGLES, transform);
        }
    }


    /** only use this for line polygons.

        better not leave triangulation to OpenGL. also, ignores texturing
    */
    void renderPolyPolygon( const ::basegfx::B2DPolyPolygon& rPolyPoly, RenderHelper *renderHelper, glm::vec4 color)
    {
        ::basegfx::B2DPolyPolygon aPolyPoly(rPolyPoly);
        if( aPolyPoly.areControlPointsUsed() )
            aPolyPoly = rPolyPoly.getAdaptiveSubdivision(COUNT_OF_ADAPTIVE_SUBDIVISION);
        for(sal_uInt32 i=0; i<aPolyPoly.count(); i++ )
        {

            const ::basegfx::B2DPolygon& rPolygon( aPolyPoly.getB2DPolygon(i) );

            const sal_uInt32 nPts=rPolygon.count();
            const sal_uInt32 nExtPts=nPts + int(rPolygon.isClosed());
            if(nExtPts>0)
            {
                std::vector<glm::vec2> vertices;
                vertices.reserve(nExtPts);
                for( sal_uInt32 j=0; j<nExtPts; j++ )
                {
                    const ::basegfx::B2DPoint& rPt( rPolygon.getB2DPoint( j % nPts ) );
                    vertices.push_back(glm::vec2(rPt.getX(),rPt.getY()));
                }
                renderHelper->renderVertexConstColor(vertices, color, GL_LINE_STRIP);
            }
        }

    }

    glm::mat4 setupState( const ::basegfx::B2DHomMatrix&   rTransform,
                     GLenum                           eSrcBlend,
                     GLenum                           eDstBlend)
    {
        const glm::mat4 aGLTransform = glm::mat4(
                (float) rTransform.get(0,0), (float) rTransform.get(1,0), 0, 0,
                (float) rTransform.get(0,1), (float) rTransform.get(1,1), 0, 0,
                0,                   0,                   1, 0,
                (float) rTransform.get(0,2), (float) rTransform.get(1,2), 0, 1);
        glEnable(GL_BLEND);
        glBlendFunc(eSrcBlend, eDstBlend);
        return aGLTransform;
    }

    void renderOSD( const std::vector<double>& rNumbers, double scale, RenderHelper *renderHelper)
    {
        double y=4.0;
        basegfx::B2DHomMatrix aTmp;
        basegfx::B2DHomMatrix aScaleShear;
        aScaleShear.shearX(-0.1);
        aScaleShear.scale(scale,scale);

        for( size_t i=0; i<rNumbers.size(); ++i )
        {
            aTmp.identity();
            aTmp.translate(0,y);
            y += 1.2*scale; //send to renderHelper

            basegfx::B2DPolyPolygon aPoly=
                basegfx::tools::number2PolyPolygon(rNumbers[i],10,3);

            aTmp=aTmp*aScaleShear;
            aPoly.transform(aTmp);
            // glColor4f(0,1,0,1);
            glm::vec4 color  = glm::vec4(1, 0, 0, 0.5);
            renderPolyPolygon(aPoly, renderHelper, color);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
