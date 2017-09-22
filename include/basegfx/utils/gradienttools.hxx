/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_BASEGFX_UTILS_GRADIENTTOOLS_HXX
#define INCLUDED_BASEGFX_UTILS_GRADIENTTOOLS_HXX

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/basegfxdllapi.h>

#include <vector>
#include <algorithm>

namespace basegfx
{
    /** Gradient definition as used in ODF 1.2

        This struct collects all data necessary for rendering ODF
        1.2-compatible gradients. Use the createXXXODFGradientInfo()
        methods below for initializing from ODF attributes.
     */
    class BASEGFX_DLLPUBLIC ODFGradientInfo
    {
    private:
        /** transformation mapping from [0,1]^2 texture coordinate
           space to [0,1]^2 shape coordinate space
         */
        B2DHomMatrix    maTextureTransform;

        /** transformation mapping from [0,1]^2 shape coordinate space
           to [0,1]^2 texture coordinate space. This is the
           transformation commonly used to create gradients from a
           scanline rasterizer (put shape u/v coordinates into it, get
           texture s/t coordinates out of it)
         */
        B2DHomMatrix    maBackTextureTransform;

        /** Aspect ratio of the gradient. Only used in drawinglayer
           for generating nested gradient polygons currently. Already
           catered for in the transformations above.
         */
        double          mfAspectRatio;

        /** Requested gradient steps to render. See the
           implementations of the getXXXGradientAlpha() methods below,
           the semantic differs slightly for the different gradient
           types.
         */
        sal_uInt32      mnSteps;

    public:
        ODFGradientInfo()
        :   maTextureTransform(),
            maBackTextureTransform(),
            mfAspectRatio(1.0),
            mnSteps(0)
        {
        }

        ODFGradientInfo(
            const B2DHomMatrix& rTextureTransform,
            double fAspectRatio,
            sal_uInt32 nSteps)
        :   maTextureTransform(rTextureTransform),
            maBackTextureTransform(),
            mfAspectRatio(fAspectRatio),
            mnSteps(nSteps)
        {
        }

        ODFGradientInfo(const ODFGradientInfo& rODFGradientInfo)
        :   maTextureTransform(rODFGradientInfo.getTextureTransform()),
            maBackTextureTransform(rODFGradientInfo.maBackTextureTransform),
            mfAspectRatio(rODFGradientInfo.getAspectRatio()),
            mnSteps(rODFGradientInfo.getSteps())
        {
        }

        ODFGradientInfo& operator=(const ODFGradientInfo& rODFGradientInfo)
        {
            maTextureTransform = rODFGradientInfo.getTextureTransform();
            maBackTextureTransform = rODFGradientInfo.maBackTextureTransform;
            mfAspectRatio = rODFGradientInfo.getAspectRatio();
            mnSteps = rODFGradientInfo.getSteps();

            return *this;
        }

        // compare operator
        bool operator==(const ODFGradientInfo& rGeoTexSvx) const;

        const B2DHomMatrix& getTextureTransform() const { return maTextureTransform; }
        const B2DHomMatrix& getBackTextureTransform() const;
        double getAspectRatio() const { return mfAspectRatio; }
        sal_uInt32 getSteps() const { return mnSteps; }

        void setTextureTransform(const B2DHomMatrix& rNew)
        {
            maTextureTransform = rNew;
            maBackTextureTransform.identity();
        }
    };

    namespace utils
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
        BASEGFX_DLLPUBLIC ODFGradientInfo createLinearODFGradientInfo(
            const B2DRange& rTargetArea,
            sal_uInt32 nSteps,
            double fBorder,
            double fAngle);


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
        BASEGFX_DLLPUBLIC double getLinearGradientAlpha(const B2DPoint& rUV,
            const ODFGradientInfo& rGradInfo);

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
        BASEGFX_DLLPUBLIC ODFGradientInfo createAxialODFGradientInfo(
            const B2DRange& rTargetArea,
            sal_uInt32 nSteps,
            double fBorder,
            double fAngle);


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
        BASEGFX_DLLPUBLIC double getAxialGradientAlpha(const B2DPoint& rUV,
            const ODFGradientInfo& rGradInfo);

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
        BASEGFX_DLLPUBLIC ODFGradientInfo createRadialODFGradientInfo(
            const B2DRange& rTargetArea,
            const B2DVector& rOffset,
            sal_uInt32 nSteps,
            double fBorder);


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
        BASEGFX_DLLPUBLIC double getRadialGradientAlpha(const B2DPoint& rUV,
            const ODFGradientInfo& rGradInfo);

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
        BASEGFX_DLLPUBLIC ODFGradientInfo createEllipticalODFGradientInfo(
            const B2DRange& rTargetArea,
            const B2DVector& rOffset,
            sal_uInt32 nSteps,
            double fBorder,
            double fAngle);


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
        BASEGFX_DLLPUBLIC double getEllipticalGradientAlpha(const B2DPoint& rUV,
            const ODFGradientInfo& rGradInfo);

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
        BASEGFX_DLLPUBLIC ODFGradientInfo createSquareODFGradientInfo(
            const B2DRange& rTargetArea,
            const B2DVector& rOffset,
            sal_uInt32 nSteps,
            double fBorder,
            double fAngle);


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
        BASEGFX_DLLPUBLIC double getSquareGradientAlpha(const B2DPoint& rUV,
            const ODFGradientInfo& rGradInfo);

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
        BASEGFX_DLLPUBLIC ODFGradientInfo createRectangularODFGradientInfo(
            const B2DRange& rTargetArea,
            const B2DVector& rOffset,
            sal_uInt32 nSteps,
            double fBorder,
            double fAngle);


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
        BASEGFX_DLLPUBLIC double getRectangularGradientAlpha(const B2DPoint& rUV,
            const ODFGradientInfo& rGradInfo);
    }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
