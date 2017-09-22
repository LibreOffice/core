/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <epoxy/gl.h>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontriangulator.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <basegfx/utils/tools.hxx>
#include <com/sun/star/rendering/ARGBColor.hpp>
#include <tools/diagnose_ex.h>

#include "ogl_canvastools.hxx"

using namespace ::com::sun::star;

namespace oglcanvas
{
    /// triangulates polygon before
    void renderComplexPolyPolygon( const ::basegfx::B2DPolyPolygon& rPolyPoly )
    {
        ::basegfx::B2DPolyPolygon aPolyPoly(rPolyPoly);
        if( aPolyPoly.areControlPointsUsed() )
            aPolyPoly = rPolyPoly.getDefaultAdaptiveSubdivision();
        const ::basegfx::B2DRange& rBounds(aPolyPoly.getB2DRange());
        const double nWidth=rBounds.getWidth();
        const double nHeight=rBounds.getHeight();
        const ::basegfx::B2DPolygon& rTriangulatedPolygon(
            ::basegfx::triangulator::triangulate(aPolyPoly));

        for( sal_uInt32 i=0; i<rTriangulatedPolygon.count(); i++ )
        {
            const ::basegfx::B2DPoint& rPt( rTriangulatedPolygon.getB2DPoint(i) );
            const double s(rPt.getX()/nWidth);
            const double t(rPt.getY()/nHeight);
            glTexCoord2f(s,t); glVertex2d(rPt.getX(), rPt.getY());
        }
    }

    /** only use this for line polygons.

        better not leave triangulation to OpenGL. also, ignores texturing
    */
    void renderPolyPolygon( const ::basegfx::B2DPolyPolygon& rPolyPoly )
    {
        ::basegfx::B2DPolyPolygon aPolyPoly(rPolyPoly);
        if( aPolyPoly.areControlPointsUsed() )
            aPolyPoly = rPolyPoly.getDefaultAdaptiveSubdivision();

        for( sal_uInt32 i=0; i<aPolyPoly.count(); i++ )
        {
            glBegin(GL_LINE_STRIP);

            const ::basegfx::B2DPolygon& rPolygon( aPolyPoly.getB2DPolygon(i) );

            const sal_uInt32 nPts=rPolygon.count();
            const sal_uInt32 nExtPts=nPts + int(rPolygon.isClosed());
            for( sal_uInt32 j=0; j<nExtPts; j++ )
            {
                const ::basegfx::B2DPoint& rPt( rPolygon.getB2DPoint( j % nPts ) );
                glVertex2d(rPt.getX(), rPt.getY());
            }

            glEnd();
        }
    }

    void setupState( const ::basegfx::B2DHomMatrix&   rTransform,
                     GLenum                           eSrcBlend,
                     GLenum                           eDstBlend,
                     const rendering::ARGBColor&      rColor )
    {
        double aGLTransform[] =
            {
                rTransform.get(0,0), rTransform.get(1,0), 0, 0,
                rTransform.get(0,1), rTransform.get(1,1), 0, 0,
                0,                   0,                   1, 0,
                rTransform.get(0,2), rTransform.get(1,2), 0, 1
            };
        glMultMatrixd(aGLTransform);

        glEnable(GL_BLEND);
        glBlendFunc(eSrcBlend, eDstBlend);

        glColor4d(rColor.Red,
                  rColor.Green,
                  rColor.Blue,
                  rColor.Alpha);

        // GL 1.2:
        // glBlendEquation( GLenum mode );
        // glBlendColor( GLclampf red, GLclampf green,GLclampf blue, GLclampf alpha );
        // glConvolutionFilter1D
        // glConvolutionFilter2D
        // glSeparableFilter2D
    }

    void renderOSD( const std::vector<double>& rNumbers, double scale )
    {
        double y=4.0;
        basegfx::B2DHomMatrix aTmp;
        basegfx::B2DHomMatrix aScaleShear;
        aScaleShear.shearX(-0.1);
        aScaleShear.scale(scale,scale);

        for(double rNumber : rNumbers)
        {
            aTmp.identity();
            aTmp.translate(0,y);
            y += 1.2*scale;

            basegfx::B2DPolyPolygon aPoly=
                basegfx::utils::number2PolyPolygon(rNumber,10,3);

            aTmp=aTmp*aScaleShear;
            aPoly.transform(aTmp);

            glColor4f(0,1,0,1);
            renderPolyPolygon(aPoly);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
