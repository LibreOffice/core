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
#include <com/sun/star/awt/ColorStopSequence.hdl>

namespace com { namespace sun { namespace star { namespace uno { class Any; } } } }
namespace com { namespace sun { namespace star { namespace awt { struct Gradient2; } } } }
namespace basegfx { class B2DRange; }

namespace basegfx
{
    /* MCGR: Provide ColorStop definition

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

        These definitions are packed in a std::vector<ColorStop> ColorStops,
        see typedef below.
    */
    class UNLESS_MERGELIBS(BASEGFX_DLLPUBLIC) ColorStop
    {
    private:
        // offset in the range of [0.0 .. 1.0]
        double mfStopOffset;

        // RGB color of ColorStop entry
        BColor maStopColor;

    public:
        // constructor - defaults are needed to have a default constructor
        // e.g. for usage in std::vector::insert (even when only reducing)
        // ensure [0.0 .. 1.0] range for mfStopOffset
        ColorStop(double fStopOffset = 0.0, const BColor& rStopColor = BColor())
            : mfStopOffset(fStopOffset)
            , maStopColor(rStopColor)
        {
            // NOTE: I originally *corrected* mfStopOffset here by using
            //   mfStopOffset(std::max(0.0, std::min(fOffset, 1.0)))
            // While that is formally correct, it moves an invalid
            // entry to 0.0 or 1.0, thus creating additional wrong
            // Start/EndColor entries. That may then 'overlay' the
            // correct entry when corrections are applied to the
            // vector of entries (see sortAndCorrectColorStops)
            // which leads to getting the wanted Start/EndColor
            // to be factically deleted, what is an error.
        }

        double getStopOffset() const { return mfStopOffset; }
        const BColor& getStopColor() const { return maStopColor; }

        // needed for std::sort
        bool operator<(const ColorStop& rCandidate) const
        {
            return getStopOffset() < rCandidate.getStopOffset();
        }

        bool operator==(const ColorStop& rCandidate) const
        {
            return getStopOffset() == rCandidate.getStopOffset() && getStopColor() == rCandidate.getStopColor();
        }
    };

    /* MCGR: Provide ColorStops definition to the FillGradientAttribute

        This array should be sorted ascending by offsets, from lowest to
        highest. Since all the primitive data definition where it is used
        is read-only, this can/will be guaranteed by forcing/checking this
        in the constructor, see ::FillGradientAttribute
    */
    typedef std::vector<ColorStop> ColorStops;

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

    // helper data struct to support buffering entries in
    // gradient texture mapping, see usages for more info
    struct UNLESS_MERGELIBS(BASEGFX_DLLPUBLIC) ColorStopRange
    {
        basegfx::BColor     maColorStart;
        basegfx::BColor     maColorEnd;
        double              mfOffsetStart;
        double              mfOffsetEnd;

        ColorStopRange()
        : maColorStart()
        , maColorEnd()
        , mfOffsetStart(0.0)
        , mfOffsetEnd(0.0)
        {}
    };

    namespace utils
    {
        /// Tooling method to fill awt::Gradient2 from data contained in the given Any
        BASEGFX_DLLPUBLIC bool fillGradient2FromAny(
            com::sun::star::awt::Gradient2& rGradient,
            const com::sun::star::uno::Any& rVal);

        /* Tooling method to extract data from given awt::Gradient2
           to ColorStops, doing some corrections, partitally based
           on given SingleColor.
           This will do quite some preparations for the gradient
           as follows:
           - It will check for single color (resetting rSingleColor when
             this is the case) and return with empty ColorStops
           - It will blend ColorStops to Intensity if StartIntensity/
             EndIntensity != 100 is set in awt::Gradient2, so applying
             that value(s) to the gadient directly
           - It will adapt to Border if Border != 0 is set at the
             given awt::Gradient2, so applying that value to the gadient
             directly
        */
        BASEGFX_DLLPUBLIC void prepareColorStops(
            const com::sun::star::awt::Gradient2& rGradient,
            ColorStops& rColorStops,
            BColor& rSingleColor);

        /* Tooling method to synchronize the given ColorStops.
           The intention is that a color GradientStops and an
           alpha/transparence GradientStops gets synchronized
           for export.
           Fo the corrections the single values for color and
           alpha may be used, e.g. when ColorStops is given
           and not empty, but AlphaStops is empty, it will get
           sycronized so that it will have the same number and
           offsets in AlphaStops as in ColorStops, but with
           the given SingleAlpha as value.
           At return it guarantees that both have the same
           number of entries with the same StopOffsets, so
           that synchonized pair of ColorStops can e.g. be used
           to export a Gradient with defined/adapted alpha
           being 'coupled' indirectly using the
           'FillTransparenceGradient' method (at import time).
        */
        BASEGFX_DLLPUBLIC void synchronizeColorStops(
            ColorStops& rColorStops,
            ColorStops& rAlphaStops,
            const BColor& rSingleColor,
            const BColor& rSingleAlpha);

        /* Tooling method to linearly blend the Colors contained in
           a given ColorStop vector against a given Color using the
           given intensity values.
           The intensity values fStartIntensity, fEndIntensity are
           in the range of [0.0 .. 1.0] and describe how much the
           blend is supposed to be done at the start color position
           and the end color position resprectively, where 0.0 means
           to fully use the given BlendColor, 1.0 means to not change
           the existing color in the ColorStop.
           Every color entry in the given ColorStop is blended
           relative to it's StopPosition, interpolating the
           given intensities with the range [0.0 .. 1.0] to do so.
        */
        BASEGFX_DLLPUBLIC void blendColorStopsToIntensity(ColorStops& rColorStops, double fStartIntensity, double fEndIntensity, const basegfx::BColor& rBlendColor);

        /* Tooling method to check if a ColorStop vector is defined
           by a single color. It returns true if this is the case.
           If true is returned, rSingleColor contains that single
           color for convenience.
           NOTE: If no ColorStop is defined, a fallback to BColor-default
                 (which is black) and true will be returned
        */
        BASEGFX_DLLPUBLIC bool isSingleColor(const ColorStops& rColorStops, BColor& rSingleColor);

        /* Tooling method to reverse ColorStops, including offsets.
           When also mirroring offsets a valid sort keeps valid.
        */
        BASEGFX_DLLPUBLIC void reverseColorStops(ColorStops& rColorStops);

        /* Tooling method to convert UNO API data to ColorStops.
           This will try to extract ColorStop data from the given
           awt::Gradient2.
        */
        BASEGFX_DLLPUBLIC void fillColorStopsFromGradient2(ColorStops& rColorStops, const com::sun::star::awt::Gradient2& rGradient);

        /* Tooling method to convert UNO API data to ColorStops.
           This will try to extract ColorStop data from the given
           Any, so if it's of type awt::Gradient2 that data will be
           extracted, converted and copied into the given ColorStops.
        */
        BASEGFX_DLLPUBLIC void fillColorStopsFromAny(ColorStops& rColorStops, const css::uno::Any& rVal);

        /* Tooling method to fill a awt::ColorStopSequence with
           the data from the given ColorStops. This is used in
           UNO API implementations.
        */
        BASEGFX_DLLPUBLIC void fillColorStopSequenceFromColorStops(css::awt::ColorStopSequence& rColorStopSequence, const ColorStops& rColorStops);

        /* Tooling method that allows to replace the StartColor in a
           vector of ColorStops. A vector in 'ordered state' is expected,
           so you may use/have used sortAndCorrectColorStops, see below.
           This method is for convenience & backwards compatibility, please
           think about handling multi-colored gradients directly.
        */
        BASEGFX_DLLPUBLIC void replaceStartColor(ColorStops& rColorStops, const BColor& rStart);

        /* Tooling method that allows to replace the EndColor in a
           vector of ColorStops. A vector in 'ordered state' is expected,
           so you may use/have used sortAndCorrectColorStops, see below.
           This method is for convenience & backwards compatibility, please
           think about handling multi-colored gradients directly.
        */
        BASEGFX_DLLPUBLIC void replaceEndColor(ColorStops& rColorStops, const BColor& rEnd);

        // Tooling method to quickly create a ColorStop vector for a given set of Start/EndColor
        BASEGFX_DLLPUBLIC ColorStops createColorStopsFromStartEndColor(const BColor& rStart, const BColor& rEnd);

        /* Tooling method to guarantee sort and correctness for
           the given ColorStops vector.
           A vector fulfilling these conditions is called to be
           in 'ordered state'.

           At return, the following conditions are guaranteed:
           - contains no ColorStops with offset < 0.0 (will
             be removed)
           - contains no ColorStops with offset > 1.0 (will
             be removed)
           - ColorStops with identical offsets are now allowed
           - will be sorted from lowest offset to highest

           Some more notes:
           - It can happen that the result is empty
           - It is allowed to have consecutive entries with
             the same color, this represents single-color
             regions inside the gradient
           - A entry with 0.0 is not required or forced, so
             no 'StartColor' is technically required
           - A entry with 1.0 is not required or forced, so
             no 'EndColor' is technically required

           All this is done in one run (sort + O(N)) without
           creating a copy of the data in any form
        */
        BASEGFX_DLLPUBLIC void sortAndCorrectColorStops(ColorStops& rColorStops);

        /* Helper to grep the correct ColorStop out of
           ColorStops and interpolate as needed for given
           relative value in fScaler in the range of [0.0 .. 1.0].
           It also takes care of evtl. given RequestedSteps.
        */
        BASEGFX_DLLPUBLIC BColor modifyBColor(
            const ColorStops& rColorStops,
            double fScaler,
            sal_uInt32 nRequestedSteps,
            ColorStopRange& rLastColorStopRange);

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
