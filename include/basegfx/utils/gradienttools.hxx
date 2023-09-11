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

#pragma once

#include <config_options.h>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/color/bcolor.hxx>
#include <utility>
#include <basegfx/basegfxdllapi.h>
#include <basegfx/utils/bgradient.hxx>
#include <osl/endian.h>

namespace basegfx { class B2DRange; }

namespace
{
    /* Internal helper to convert ::Color from tools::color.hxx to BColor
        without the need to link against tools library. Be on the
        safe side by using the same union
    */
    struct ColorToBColorConverter
    {
        union {
            sal_uInt32 mValue;
            struct {
#ifdef OSL_BIGENDIAN
                sal_uInt8 T;
                sal_uInt8 R;
                sal_uInt8 G;
                sal_uInt8 B;
#else
                sal_uInt8 B;
                sal_uInt8 G;
                sal_uInt8 R;
                sal_uInt8 T;
#endif
            };
        };

        ColorToBColorConverter GetRGBColor() const
        {
            return {R, G, B};
        }

        ColorToBColorConverter(sal_uInt32 nColor)
        : mValue(nColor)
        { T=0; }

        constexpr ColorToBColorConverter(sal_uInt8 nRed, sal_uInt8 nGreen, sal_uInt8 nBlue)
        : mValue(sal_uInt32(nBlue) | (sal_uInt32(nGreen) << 8) | (sal_uInt32(nRed) << 16))
        {}

        explicit ColorToBColorConverter(const basegfx::BColor& rBColor)
        : ColorToBColorConverter(
            sal_uInt8(std::lround(rBColor.getRed() * 255.0)),
            sal_uInt8(std::lround(rBColor.getGreen() * 255.0)),
            sal_uInt8(std::lround(rBColor.getBlue() * 255.0)))
        {}

        basegfx::BColor getBColor() const
        {
            return basegfx::BColor(R / 255.0, G / 255.0, B / 255.0);
        }

        constexpr explicit operator sal_Int32() const
        {
            return sal_Int32(mValue);
        }

        constexpr explicit operator sal_uInt32() const
        {
            return mValue;
        }
    };
}

namespace basegfx
{
    /** Gradient definition as used in ODF 1.2

        This struct collects all data necessary for rendering ODF
        1.2-compatible gradients. Use the createXXXODFGradientInfo()
        methods below for initializing from ODF attributes.
     */
    class UNLESS_MERGELIBS(BASEGFX_DLLPUBLIC) ODFGradientInfo
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
        sal_uInt32      mnRequestedSteps;

    public:
        ODFGradientInfo()
        :   mfAspectRatio(1.0),
            mnRequestedSteps(0)
        {
        }

        ODFGradientInfo(
            B2DHomMatrix aTextureTransform,
            double fAspectRatio,
            sal_uInt32 nRequestedSteps)
        :   maTextureTransform(std::move(aTextureTransform)),
            mfAspectRatio(fAspectRatio),
            mnRequestedSteps(nRequestedSteps)
        {
        }

        ODFGradientInfo(const ODFGradientInfo& rODFGradientInfo)
        :   maTextureTransform(rODFGradientInfo.getTextureTransform()),
            maBackTextureTransform(rODFGradientInfo.maBackTextureTransform),
            mfAspectRatio(rODFGradientInfo.getAspectRatio()),
            mnRequestedSteps(rODFGradientInfo.getRequestedSteps())
        {
        }

        ODFGradientInfo& operator=(const ODFGradientInfo& rODFGradientInfo)
        {
            maTextureTransform = rODFGradientInfo.getTextureTransform();
            maBackTextureTransform = rODFGradientInfo.maBackTextureTransform;
            mfAspectRatio = rODFGradientInfo.getAspectRatio();
            mnRequestedSteps = rODFGradientInfo.getRequestedSteps();

            return *this;
        }

        // compare operator
        bool operator==(const ODFGradientInfo& rGeoTexSvx) const;

        const B2DHomMatrix& getTextureTransform() const { return maTextureTransform; }
        const B2DHomMatrix& getBackTextureTransform() const;
        double getAspectRatio() const { return mfAspectRatio; }
        sal_uInt32 getRequestedSteps() const { return mnRequestedSteps; }

        void setTextureTransform(const B2DHomMatrix& rNew)
        {
            maTextureTransform = rNew;
            maBackTextureTransform.identity();
        }
    };

    namespace utils
    {
        /* Tooling method to extract data from given BGradient
           to ColorStops, doing some corrections, partially based
           on given SingleColor.
           This is used for export preparations in case these exports
           do neither support Start/EndIntensity nor Border settings,
           both will be eliminated if possible (see below).
           The BGradient rGradient and BColorStops& rColorStops
           are both return parameters and may be changed.
           This will do quite some preparations for the gradient
           as follows:
           - It will check for single color (resetting rSingleColor when
             this is the case) and return with empty ColorStops
           - It will blend ColorStops to Intensity if StartIntensity/
             EndIntensity != 100 is set in BGradient, so applying
             that value(s) to the gradient directly
           - It will adapt to Border if Border != 0 is set at the
             given BGradient, so applying that value to the gradient
             directly
        */
        BASEGFX_DLLPUBLIC void prepareColorStops(
            const basegfx::BGradient& rGradient,
            BColorStops& rColorStops,
            BColor& rSingleColor);

        /* Tooling method to synchronize the given ColorStops.
           The intention is that a color GradientStops and an
           alpha/transparence GradientStops gets synchronized
           for export.
           For the corrections the single values for color and
           alpha may be used, e.g. when ColorStops is given
           and not empty, but AlphaStops is empty, it will get
           synchronized so that it will have the same number and
           offsets in AlphaStops as in ColorStops, but with
           the given SingleAlpha as value.
           At return it guarantees that both have the same
           number of entries with the same StopOffsets, so
           that synchronized pair of ColorStops can e.g. be used
           to export a Gradient with defined/adapted alpha
           being 'coupled' indirectly using the
           'FillTransparenceGradient' method (at import time).
        */
        BASEGFX_DLLPUBLIC void synchronizeColorStops(
            BColorStops& rColorStops,
            BColorStops& rAlphaStops,
            const BColor& rSingleColor,
            const BColor& rSingleAlpha);

        /* Helper to calculate numberOfSteps needed to represent
           gradient for the given two colors:
           - to define only based on color distance, give 0 == nRequestedSteps
               as wanted value, so color distance will be used
           - if a wanted value of nRequestedSteps is given, it gets synched
               against the maximum number of steps defined by the color
               distance of the two colors
           - a minimum result of 1 is returned which means a single
               step -> no real gradient
        */
        BASEGFX_DLLPUBLIC sal_uInt32 calculateNumberOfSteps(
            sal_uInt32 nRequestedSteps,
            const BColor& rStart,
            const BColor& rEnd);

        /** Create matrix for ODF's linear gradient definition

            Note that odf linear gradients are varying in y direction.

            @param o_rGradientInfo
            Receives the calculated texture transformation matrix (for
            use with standard [0,1]x[0,1] texture coordinates)

            @param rTargetArea
            Output area, needed for aspect ratio calculations and
            texture transformation

            @param nRequestedSteps
            Number of gradient steps (from ODF)

            @param fBorder
            Width of gradient border (from ODF)

            @param fAngle
            Gradient angle (from ODF)
         */
        BASEGFX_DLLPUBLIC ODFGradientInfo createLinearODFGradientInfo(
            const B2DRange& rTargetArea,
            sal_uInt32 nRequestedSteps,
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

            @param nRequestedSteps
            Number of gradient steps (from ODF)

            @param fBorder
            Width of gradient border (from ODF)

            @param fAngle
            Gradient angle (from ODF)
         */
        BASEGFX_DLLPUBLIC ODFGradientInfo createAxialODFGradientInfo(
            const B2DRange& rTargetArea,
            sal_uInt32 nRequestedSteps,
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

            @param nRequestedSteps
            Number of gradient steps (from ODF)

            @param fBorder
            Width of gradient border (from ODF)

            @param fAngle
            Gradient angle (from ODF)
         */
        BASEGFX_DLLPUBLIC ODFGradientInfo createRadialODFGradientInfo(
            const B2DRange& rTargetArea,
            const B2DVector& rOffset,
            sal_uInt32 nRequestedSteps,
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

            @param nRequestedSteps
            Number of gradient steps (from ODF)

            @param fBorder
            Width of gradient border (from ODF)

            @param fAngle
            Gradient angle (from ODF)
         */
        BASEGFX_DLLPUBLIC ODFGradientInfo createEllipticalODFGradientInfo(
            const B2DRange& rTargetArea,
            const B2DVector& rOffset,
            sal_uInt32 nRequestedSteps,
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

            @param nRequestedSteps
            Number of gradient steps (from ODF)

            @param fBorder
            Width of gradient border (from ODF)

            @param fAngle
            Gradient angle (from ODF)
         */
        BASEGFX_DLLPUBLIC ODFGradientInfo createSquareODFGradientInfo(
            const B2DRange& rTargetArea,
            const B2DVector& rOffset,
            sal_uInt32 nRequestedSteps,
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

            @param nRequestedSteps
            Number of gradient steps (from ODF)

            @param fBorder
            Width of gradient border (from ODF)

            @param fAngle
            Gradient angle (from ODF)
         */
        BASEGFX_DLLPUBLIC ODFGradientInfo createRectangularODFGradientInfo(
            const B2DRange& rTargetArea,
            const B2DVector& rOffset,
            sal_uInt32 nRequestedSteps,
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
