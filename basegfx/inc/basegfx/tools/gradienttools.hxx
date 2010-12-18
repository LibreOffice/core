/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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

#ifndef _BGFX_TOOLS_GRADIENTTOOLS_HXX
#define _BGFX_TOOLS_GRADIENTTOOLS_HXX

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>

#include <vector>
#include <algorithm>

namespace basegfx
{
    /** Gradient definition as used in ODF 1.2

        This struct collects all data necessary for rendering ODF
        1.2-compatible gradients. Use the createXXXODFGradientInfo()
        methods below for initializing from ODF attributes.
     */
    struct ODFGradientInfo
    {
        /** transformation mapping from [0,1]^2 texture coordinate
           space to [0,1]^2 shape coordinate space
         */
        B2DHomMatrix maTextureTransform;

        /** transformation mapping from [0,1]^2 shape coordinate space
           to [0,1]^2 texture coordinate space. This is the
           transformation commonly used to create gradients from a
           scanline rasterizer (put shape u/v coordinates into it, get
           texture s/t coordinates out of it)
         */
        B2DHomMatrix maBackTextureTransform;

        /** Aspect ratio of the gradient. Only used in drawinglayer
           for generating nested gradient polygons currently. Already
           catered for in the transformations above.
         */
        double       mfAspectRatio;

        /** Requested gradient steps to render. See the
           implementations of the getXXXGradientAlpha() methods below,
           the semantic differs slightly for the different gradient
           types.
         */
        sal_uInt32   mnSteps;
    };

    namespace tools
    {
        /** Create matrix for ODF's linear gradient definition

            Note that odf linear gradients are varying in y direction.

            @param o_rGradientInfo
            Receives the calculated texture transformation matrix (for
            use with standard [0,1]x[0,1] texture coordinates)

            @param rTargetArea
            Output area, needed for aspect ratio calculations and
            texture transformation

            @param nSteps
            Number of gradient steps (from ODF)

            @param fBorder
            Width of gradient border (from ODF)

            @param fAngle
            Gradient angle (from ODF)
         */
        ODFGradientInfo& createLinearODFGradientInfo(ODFGradientInfo& o_rGradientInfo,
                                                     const B2DRange&  rTargetArea,
                                                     sal_uInt32       nSteps,
                                                     double           fBorder,
                                                     double           fAngle);

        /** Calculate linear gradient blend value

            This method generates you the lerp alpha value for
            blending linearly between gradient start and end color,
            according to the formula (startCol*(1.0-alpha) + endCol*alpha)

            @param rUV
            Current uv coordinate. Values outside [0,1] will be
            clamped. Assumes gradient color varies along the y axis.

            @param rGradInfo
            Gradient info, for transformation and number of steps
         */
        inline double getLinearGradientAlpha(const B2DPoint&        rUV,
                                             const ODFGradientInfo& rGradInfo )
        {
            const B2DPoint aCoor(rGradInfo.maBackTextureTransform * rUV);
            const double t(clamp(aCoor.getY(), 0.0, 1.0));
            const sal_uInt32 nSteps(rGradInfo.mnSteps);

            if(nSteps > 2L && nSteps < 128L)
                return floor(t * nSteps) / double(nSteps + 1L);

            return t;
        }

        /** Create matrix for ODF's axial gradient definition

            Note that odf axial gradients are varying in y
            direction. Note further that you can map the axial
            gradient to a linear gradient (in case you want or need to
            avoid an extra gradient renderer), by using
            createLinearODFGradientInfo() instead, shifting the
            resulting texture transformation by 0.5 to the top and
            appending the same stop colors again, but mirrored.

            @param o_rGradientInfo
            Receives the calculated texture transformation matrix (for
            use with standard [0,1]x[0,1] texture coordinates)

            @param rTargetArea
            Output area, needed for aspect ratio calculations and
            texture transformation

            @param nSteps
            Number of gradient steps (from ODF)

            @param fBorder
            Width of gradient border (from ODF)

            @param fAngle
            Gradient angle (from ODF)
         */
        ODFGradientInfo& createAxialODFGradientInfo(ODFGradientInfo& o_rGradientInfo,
                                                    const B2DRange&  rTargetArea,
                                                    sal_uInt32       nSteps,
                                                    double           fBorder,
                                                    double           fAngle);

        /** Calculate axial gradient blend value

            This method generates you the lerp alpha value for
            blending linearly between gradient start and end color,
            according to the formula (startCol*(1.0-alpha) + endCol*alpha)

            @param rUV
            Current uv coordinate. Values outside [0,1] will be
            clamped. Assumes gradient color varies along the y axis.

            @param rGradInfo
            Gradient info, for transformation and number of steps
         */
        inline double getAxialGradientAlpha(const B2DPoint&        rUV,
                                            const ODFGradientInfo& rGradInfo )
        {
            const B2DPoint   aCoor(rGradInfo.maBackTextureTransform * rUV);
            const double     t(clamp(fabs(aCoor.getY()), 0.0, 1.0));
            const sal_uInt32 nSteps(rGradInfo.mnSteps);
            const double     fInternalSteps((nSteps * 2L) - 1L);

            if(nSteps > 2L && nSteps < 128L)
                return floor(((t * fInternalSteps) + 1.0) / 2.0) / double(nSteps - 1L);

            return t;
        }

        /** Create matrix for ODF's radial gradient definition

            @param o_rGradientInfo
            Receives the calculated texture transformation matrix (for
            use with standard [0,1]x[0,1] texture coordinates)

            @param rTargetArea
            Output area, needed for aspect ratio calculations and
            texture transformation

            @param rOffset
            Gradient offset value (from ODF)

            @param nSteps
            Number of gradient steps (from ODF)

            @param fBorder
            Width of gradient border (from ODF)

            @param fAngle
            Gradient angle (from ODF)
         */
        ODFGradientInfo& createRadialODFGradientInfo(ODFGradientInfo& o_rGradientInfo,
                                                     const B2DRange&  rTargetArea,
                                                     const B2DVector& rOffset,
                                                     sal_uInt32       nSteps,
                                                     double           fBorder);

        /** Calculate radial gradient blend value

            This method generates you the lerp alpha value for
            blending linearly between gradient start and end color,
            according to the formula (startCol*(1.0-alpha) + endCol*alpha)

            @param rUV
            Current uv coordinate. Values outside [0,1] will be
            clamped.

            @param rGradInfo
            Gradient info, for transformation and number of steps
         */
        inline double getRadialGradientAlpha(const B2DPoint&        rUV,
                                             const ODFGradientInfo& rGradInfo )
        {
            const B2DPoint aCoor(rGradInfo.maBackTextureTransform * rUV);
            const double   fDist(
                clamp(aCoor.getX() * aCoor.getX() + aCoor.getY() * aCoor.getY(),
                      0.0,
                      1.0));

            const double t(1.0 - sqrt(fDist));
            const sal_uInt32 nSteps(rGradInfo.mnSteps);

            if(nSteps > 2L && nSteps < 128L)
                return floor(t * nSteps) / double(nSteps - 1L);

            return t;
        }

        /** Create matrix for ODF's elliptical gradient definition

            @param o_rGradientInfo
            Receives the calculated texture transformation matrix (for
            use with standard [0,1]x[0,1] texture coordinates)

            @param rTargetArea
            Output area, needed for aspect ratio calculations and
            texture transformation

            @param rOffset
            Gradient offset value (from ODF)

            @param nSteps
            Number of gradient steps (from ODF)

            @param fBorder
            Width of gradient border (from ODF)

            @param fAngle
            Gradient angle (from ODF)
         */
        ODFGradientInfo& createEllipticalODFGradientInfo(ODFGradientInfo& o_rGradientInfo,
                                                         const B2DRange&  rTargetArea,
                                                         const B2DVector& rOffset,
                                                         sal_uInt32       nSteps,
                                                         double           fBorder,
                                                         double           fAngle);

        /** Calculate elliptical gradient blend value

            This method generates you the lerp alpha value for
            blending linearly between gradient start and end color,
            according to the formula (startCol*(1.0-alpha) + endCol*alpha)

            @param rUV
            Current uv coordinate. Values outside [0,1] will be
            clamped.

            @param rGradInfo
            Gradient info, for transformation and number of steps
         */
        inline double getEllipticalGradientAlpha(const B2DPoint&        rUV,
                                                 const ODFGradientInfo& rGradInfo )
        {
            return getRadialGradientAlpha(rUV,rGradInfo); // only matrix setup differs
        }

        /** Create matrix for ODF's square gradient definition

            @param o_rGradientInfo
            Receives the calculated texture transformation matrix (for
            use with standard [0,1]x[0,1] texture coordinates)

            @param rTargetArea
            Output area, needed for aspect ratio calculations and
            texture transformation

            @param rOffset
            Gradient offset value (from ODF)

            @param nSteps
            Number of gradient steps (from ODF)

            @param fBorder
            Width of gradient border (from ODF)

            @param fAngle
            Gradient angle (from ODF)
         */
        ODFGradientInfo& createSquareODFGradientInfo(ODFGradientInfo& o_rGradientInfo,
                                                     const B2DRange&  rTargetArea,
                                                     const B2DVector& rOffset,
                                                     sal_uInt32       nSteps,
                                                     double           fBorder,
                                                     double           fAngle);

        /** Calculate square gradient blend value

            This method generates you the lerp alpha value for
            blending linearly between gradient start and end color,
            according to the formula (startCol*(1.0-alpha) + endCol*alpha)

            @param rUV
            Current uv coordinate. Values outside [0,1] will be
            clamped.

            @param rGradInfo
            Gradient info, for transformation and number of steps
         */
        inline double getSquareGradientAlpha(const B2DPoint&        rUV,
                                             const ODFGradientInfo& rGradInfo )
        {
            const B2DPoint aCoor(rGradInfo.maBackTextureTransform * rUV);
            const double   fAbsX(fabs(aCoor.getX()));
            const double   fAbsY(fabs(aCoor.getY()));

            if(fTools::moreOrEqual(fAbsX, 1.0) || fTools::moreOrEqual(fAbsY, 1.0))
                return 0.0;

            const double t(1.0 - (fAbsX > fAbsY ? fAbsX : fAbsY));
            const sal_uInt32 nSteps(rGradInfo.mnSteps);

            if(nSteps > 2L && nSteps < 128L)
                return floor(t * nSteps) / double(nSteps - 1L);

            return t;
        }

        /** Create matrix for ODF's rectangular gradient definition

            @param o_rGradientInfo
            Receives the calculated texture transformation matrix (for
            use with standard [0,1]x[0,1] texture coordinates)

            @param rTargetArea
            Output area, needed for aspect ratio calculations and
            texture transformation

            @param rOffset
            Gradient offset value (from ODF)

            @param nSteps
            Number of gradient steps (from ODF)

            @param fBorder
            Width of gradient border (from ODF)

            @param fAngle
            Gradient angle (from ODF)
         */
        ODFGradientInfo& createRectangularODFGradientInfo(ODFGradientInfo& o_rGradientInfo,
                                                          const B2DRange&  rTargetArea,
                                                          const B2DVector& rOffset,
                                                          sal_uInt32       nSteps,
                                                          double           fBorder,
                                                          double           fAngle);

        /** Calculate rectangular gradient blend value

            This method generates you the lerp alpha value for
            blending linearly between gradient start and end color,
            according to the formula (startCol*(1.0-alpha) + endCol*alpha)

            @param rUV
            Current uv coordinate. Values outside [0,1] will be
            clamped.

            @param rGradInfo
            Gradient info, for transformation and number of steps
         */
        inline double getRectangularGradientAlpha(const B2DPoint&        rUV,
                                                  const ODFGradientInfo& rGradInfo )
        {
            return getSquareGradientAlpha(rUV, rGradInfo); // only matrix setup differs
        }
    }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
