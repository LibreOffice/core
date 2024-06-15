/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <basegfx/color/bcolor.hxx>
#include <basegfx/basegfxdllapi.h>
#include <vector>
#include <com/sun/star/awt/GradientStyle.hpp>
#include <tools/degree.hxx>
#include <boost/property_tree/ptree_fwd.hpp>

namespace basegfx
{
class BColorModifierStack;
}

namespace basegfx
{
/* MCGR: Provide ColorStop definition

        This is the needed combination of offset and color:

        Offset is defined as:
        - being in the range of [0.0 .. 1.0] (unit range)
          - offsets outside are an error
        - lowest/1st value equivalent to StartColor
        - highest/last value equivalent to EndColor
        - missing 0.0/1.0 entries are allowed
        - at least one value (usually 0.0, StartColor) is required
            - this allows to avoid massive testing in all places where
              this data has to be accessed

        Color is defined as:
        - RGB with unit values [0.0 .. 1.0]

        These definitions are packed in a std::vector<ColorStop> ColorStops,
        see typedef below.
    */
class BASEGFX_DLLPUBLIC BColorStop
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
    BColorStop(double fStopOffset = 0.0, const BColor& rStopColor = BColor())
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
    bool operator<(const BColorStop& rCandidate) const
    {
        return getStopOffset() < rCandidate.getStopOffset();
    }

    bool operator==(const BColorStop& rCandidate) const
    {
        return getStopOffset() == rCandidate.getStopOffset()
               && getStopColor() == rCandidate.getStopColor();
    }

    bool operator!=(const BColorStop& rCandidate) const { return !(*this == rCandidate); }
};

/* MCGR: Provide ColorStops definition to the FillGradientAttribute

        This array should be sorted ascending by offsets, from lowest to
        highest. Since all the primitive data definition where it is used
        is read-only, this can/will be guaranteed by forcing/checking this
        in the constructor, see ::FillGradientAttribute
    */
class BASEGFX_DLLPUBLIC BColorStops final : public std::vector<BColorStop>
{
public:
    explicit BColorStops()
        : vector()
    {
    }
    BColorStops(const BColorStops& other)
        : vector(other)
    {
    }
    BColorStops(BColorStops&& other) noexcept
        : vector(std::move(other))
    {
    }
    BColorStops(std::initializer_list<BColorStop> init)
        : vector(init)
    {
    }
    BColorStops(const_iterator first, const_iterator last)
        : vector(first, last)
    {
    }

    // constructor with two colors to explicitly create a
    // BColorStops for StartColor @0.0 & EndColor @1.0
    BColorStops(const BColor& rStart, const BColor& rEnd);

    BColorStops& operator=(const BColorStops& r)
    {
        vector::operator=(r);
        return *this;
    }
    BColorStops& operator=(BColorStops&& r) noexcept
    {
        vector::operator=(std::move(r));
        return *this;
    }

    // helper data struct to support buffering entries in
    // gradient texture mapping, see usages for more info
    struct BColorStopRange
    {
        basegfx::BColor maColorStart;
        basegfx::BColor maColorEnd;
        double mfOffsetStart;
        double mfOffsetEnd;

        BColorStopRange()
            : maColorStart()
            , maColorEnd()
            , mfOffsetStart(0.0)
            , mfOffsetEnd(0.0)
        {
        }
    };

    /* Helper to grep the correct ColorStop out of
           ColorStops and interpolate as needed for given
           relative value in fPosition in the range of [0.0 .. 1.0].
           It also takes care of evtl. given RequestedSteps.
        */
    BColor getInterpolatedBColor(double fPosition, sal_uInt32 nRequestedSteps,
                                 BColorStopRange& rLastColorStopRange) const;

    /* Tooling method that allows to replace the StartColor in a
           vector of ColorStops. A vector in 'ordered state' is expected,
           so you may use/have used sortAndCorrect.
           This method is for convenience & backwards compatibility, please
           think about handling multi-colored gradients directly.
        */
    void replaceStartColor(const BColor& rStart);

    /* Tooling method that allows to replace the EndColor in a
           vector of ColorStops. A vector in 'ordered state' is expected,
           so you may use/have used sortAndCorrect.
           This method is for convenience & backwards compatibility, please
           think about handling multi-colored gradients directly.
        */
    void replaceEndColor(const BColor& rEnd);

    /* Tooling method to linearly blend the Colors contained in
           a given ColorStop vector against a given Color using the
           given intensity values.
           The intensity values fStartIntensity, fEndIntensity are
           in the range of [0.0 .. 1.0] and describe how much the
           blend is supposed to be done at the start color position
           and the end color position respectively, where 0.0 means
           to fully use the given BlendColor, 1.0 means to not change
           the existing color in the ColorStop.
           Every color entry in the given ColorStop is blended
           relative to it's StopPosition, interpolating the
           given intensities with the range [0.0 .. 1.0] to do so.
        */
    void blendToIntensity(double fStartIntensity, double fEndIntensity, const BColor& rBlendColor);

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
    void sortAndCorrect();

    // check if we need last-ColorStop-correction. This returns true if the last
    // two ColorStops have the same offset but different Colors. In that case the
    // tessellation for gradients does have to create an extra ending/closing entry
    bool checkPenultimate() const;

    /* Tooling method to check if a ColorStop vector is defined
            by a single color. It returns true if this is the case.
            If true is returned, rSingleColor contains that single
            color for convenience.
            NOTE: If no ColorStop is defined, a fallback to BColor-default
                    (which is black) and true will be returned
        */
    bool isSingleColor(BColor& rSingleColor) const;

    /* Tooling method to reverse ColorStops, including offsets.
           When also mirroring offsets a valid sort keeps valid.
        */
    void reverseColorStops();

    // createSpaceAtStart creates fOffset space at start by
    // translating/scaling all entries to the right
    void createSpaceAtStart(double fOffset);

    // removeSpaceAtStart removes fOffset space from start by
    // translating/scaling entries more or equal to fOffset
    // to the left. Entries less than fOffset will be removed
    void removeSpaceAtStart(double fOffset);

    // try to detect if an empty/no-color-change area exists
    // at the start and return offset to it. Returns 0.0 if not.
    double detectPossibleOffsetAtStart() const;

    // returns true if the color stops are symmetrical in color and offset, otherwise false.
    bool isSymmetrical() const;
    // assume that the color stops represent an Axial gradient
    // and replace with gradient stops to represent the same
    // gradient as linear gradient
    void doApplyAxial();

    // apply Steps as 'hard' color stops
    void doApplySteps(sal_uInt16 nStepCount);

    // Apply BColorModifierStack changes
    void tryToApplyBColorModifierStack(const BColorModifierStack& rBColorModifierStack);
};

class BASEGFX_DLLPUBLIC BGradient final
{
private:
    css::awt::GradientStyle eStyle;

    // MCGS: ColorStops in the range [0.0 .. 1.0], including StartColor/EndColor
    basegfx::BColorStops aColorStops;

    Degree10 nAngle;
    sal_uInt16 nBorder;
    sal_uInt16 nOfsX;
    sal_uInt16 nOfsY;
    sal_uInt16 nIntensStart;
    sal_uInt16 nIntensEnd;
    sal_uInt16 nStepCount;

    static std::string GradientStyleToString(css::awt::GradientStyle eStyle);

public:
    BGradient();
    BGradient(const basegfx::BColorStops& rColorStops,
              css::awt::GradientStyle eStyle = css::awt::GradientStyle_LINEAR,
              Degree10 nAngle = 0_deg10, sal_uInt16 nXOfs = 50, sal_uInt16 nYOfs = 50,
              sal_uInt16 nBorder = 0, sal_uInt16 nStartIntens = 100, sal_uInt16 nEndIntens = 100,
              sal_uInt16 nSteps = 0);

    bool operator==(const BGradient& rGradient) const;

    void SetGradientStyle(css::awt::GradientStyle eNewStyle) { eStyle = eNewStyle; }
    void SetColorStops(const basegfx::BColorStops& rSteps);
    void SetAngle(Degree10 nNewAngle) { nAngle = nNewAngle; }
    void SetBorder(sal_uInt16 nNewBorder) { nBorder = nNewBorder; }
    void SetXOffset(sal_uInt16 nNewOffset) { nOfsX = nNewOffset; }
    void SetYOffset(sal_uInt16 nNewOffset) { nOfsY = nNewOffset; }
    void SetStartIntens(sal_uInt16 nNewIntens) { nIntensStart = nNewIntens; }
    void SetEndIntens(sal_uInt16 nNewIntens) { nIntensEnd = nNewIntens; }
    void SetSteps(sal_uInt16 nSteps) { nStepCount = nSteps; }

    css::awt::GradientStyle GetGradientStyle() const { return eStyle; }
    const basegfx::BColorStops& GetColorStops() const { return aColorStops; }
    Degree10 GetAngle() const { return nAngle; }
    sal_uInt16 GetBorder() const { return nBorder; }
    sal_uInt16 GetXOffset() const { return nOfsX; }
    sal_uInt16 GetYOffset() const { return nOfsY; }
    sal_uInt16 GetStartIntens() const { return nIntensStart; }
    sal_uInt16 GetEndIntens() const { return nIntensEnd; }
    sal_uInt16 GetSteps() const { return nStepCount; }

    boost::property_tree::ptree dumpAsJSON() const;
    static BGradient fromJSON(std::u16string_view rJSON);

    // Tooling to handle
    // - border correction/integration
    // - apply StartStopIntensity to color stops
    // - convert type from 'axial' to linear
    // - apply Steps as 'hard' color stops
    void tryToRecreateBorder(basegfx::BColorStops* pAssociatedTransparencyStops = nullptr);
    void tryToApplyBorder();
    void tryToApplyStartEndIntensity();

    // If a linear gradient is symmetrical it is converted to an axial gradient.
    // Does nothing in other cases and for other gradient types.
    void tryToConvertToAxial();
    void tryToApplyAxial();
    void tryToApplySteps();
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
