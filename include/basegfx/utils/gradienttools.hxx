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
#include <vector>

namespace basegfx { class B2DRange; }

namespace basegfx
{
    /* MCGR: Provide ColorStep definition

        This is the needed combination of offset and color:

        Offset is defined as:
        - being in the range of [0.0 .. 1.0] (unit range)
          - 0.0 being reserved for StartColor
          - 1.0 being reserved for EndColor
        - in-between offsets thus being in the range of ]0.0 .. 1.0[
        - no two equal offsets are allowed
            - this is an error
        - missing 1.0 entry (EndColor) is allowed
          - it means that EndColor == StartColor
        - at least one value (usually 0.0, StartColor) is required
            - this allows to avoid massive testing in all places where
              this data has to be accessed

        Color is defined as:
        - RGB with unit values [0.0 .. 1.0]

        These definitions are packed in a std::vector<ColorStep> ColorSteps,
        see typedef below.
    */
    class UNLESS_MERGELIBS(BASEGFX_DLLPUBLIC) ColorStep
    {
    private:
        // offset in the range of [0.0 .. 1.0], checked & force by constructor
        double mfOffset;

        // color of ColorStep entry
        BColor maColor;

    public:
        // constructor - defaults are needed to have a default constructor
        // e.g. for usage in std::vector::insert
        // ensure [0.0 .. 1.0] range for mfOffset
        ColorStep(double fOffset = 0.0, const BColor& rColor = BColor())
            : mfOffset(std::max(0.0, std::min(fOffset, 1.0)))
            , maColor(rColor)
        {
        }

        double getOffset() const { return mfOffset; }
        const BColor& getColor() const { return maColor; }

        bool operator<(const ColorStep& rCandidate) const
        {
            return getOffset() < rCandidate.getOffset();
        }

        bool operator==(const ColorStep& rCandidate) const
        {
            return getOffset() == rCandidate.getOffset() && getColor() == rCandidate.getColor();
        }
    };

    /* MCGR: Provide ColorSteps definition to the FillGradientAttribute

        This array should be sorted ascending by offsets, from lowest to
        highest. Since all the primitive data definition where it is used
        is read-only, this can/will be guaranteed by forcing/checking this
        in the constructor, see ::FillGradientAttribute
    */
    typedef std::vector<ColorStep> ColorSteps;

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
        /* Helper to grep the correct ColorStep out of
           ColorSteps and interpolate as needed for given
           relative value in fScaler in the range of [0.0 .. 1.0].
           It also takes care of evtl. given RequestedSteps.
        */
        BASEGFX_DLLPUBLIC BColor modifyBColor(
            const ColorSteps& rColorSteps,
            double fScaler,
            sal_uInt32 nRequestedSteps);

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
