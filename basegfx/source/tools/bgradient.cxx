/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <basegfx/utils/bgradient.hxx>
#include <basegfx/utils/gradienttools.hxx>
#include <com/sun/star/awt/Gradient2.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <map>

typedef std::map<OUString, OUString> StringMap;

namespace
{
css::awt::GradientStyle lcl_getStyleFromString(std::u16string_view rStyle)
{
    if (rStyle == u"LINEAR")
        return css::awt::GradientStyle_LINEAR;
    else if (rStyle == u"AXIAL")
        return css::awt::GradientStyle_AXIAL;
    else if (rStyle == u"RADIAL")
        return css::awt::GradientStyle_RADIAL;
    else if (rStyle == u"ELLIPTICAL")
        return css::awt::GradientStyle_ELLIPTICAL;
    else if (rStyle == u"SQUARE")
        return css::awt::GradientStyle_SQUARE;
    else if (rStyle == u"RECT")
        return css::awt::GradientStyle_RECT;

    return css::awt::GradientStyle_LINEAR;
}

StringMap lcl_jsonToStringMap(std::u16string_view rJSON)
{
    StringMap aArgs;
    if (rJSON.size() && rJSON[0] != '\0')
    {
        std::stringstream aStream(std::string(OUStringToOString(rJSON, RTL_TEXTENCODING_ASCII_US)));
        boost::property_tree::ptree aTree;
        boost::property_tree::read_json(aStream, aTree);

        for (const auto& rPair : aTree)
        {
            aArgs[OUString::fromUtf8(rPair.first)]
                = OUString::fromUtf8(rPair.second.get_value<std::string>("."));
        }
    }
    return aArgs;
}

basegfx::BGradient lcl_buildGradientFromStringMap(StringMap& rMap)
{
    basegfx::BGradient aGradient(
        basegfx::BColorStops(ColorToBColorConverter(rMap["startcolor"].toInt32(16)).getBColor(),
                             ColorToBColorConverter(rMap["endcolor"].toInt32(16)).getBColor()));

    aGradient.SetGradientStyle(lcl_getStyleFromString(rMap["style"]));
    aGradient.SetAngle(Degree10(rMap["angle"].toInt32()));

    return aGradient;
}
}

namespace basegfx
{
// constructor with two colors to explicitly create a
// BColorStops for a single StartColor @0.0 & EndColor @1.0
BColorStops::BColorStops(const BColor& rStart, const BColor& rEnd)
{
    emplace_back(0.0, rStart);
    emplace_back(1.0, rEnd);
}

/* Helper to grep the correct ColorStop out of
        ColorStops and interpolate as needed for given
        relative value in fPosition in the range of [0.0 .. 1.0].
        It also takes care of evtl. given RequestedSteps.
    */
BColor BColorStops::getInterpolatedBColor(double fPosition, sal_uInt32 nRequestedSteps,
                                          BColorStopRange& rLastColorStopRange) const
{
    // no color at all, done
    if (empty())
        return BColor();

    // outside range -> at start
    const double fMin(front().getStopOffset());
    if (fPosition < fMin)
        return front().getStopColor();

    // outside range -> at end
    const double fMax(back().getStopOffset());
    if (fPosition > fMax)
        return back().getStopColor();

    // special case for the 'classic' case with just two colors:
    // we can optimize that and keep the speed/resources low
    // by avoiding some calculations and an O(log(N)) array access
    if (2 == size())
    {
        // if same StopOffset use front color
        if (fTools::equal(fMin, fMax))
            return front().getStopColor();

        const basegfx::BColor aCStart(front().getStopColor());
        const basegfx::BColor aCEnd(back().getStopColor());

        // if colors are equal just return one
        if (aCStart == aCEnd)
            return aCStart;

        // calculate Steps
        const sal_uInt32 nSteps(
            basegfx::utils::calculateNumberOfSteps(nRequestedSteps, aCStart, aCEnd));

        // we need to extend the interpolation to the local
        // range of ColorStops. Despite having two ColorStops
        // these are not necessarily at 0.0 and 1.0, so may be
        // not the classical Start/EndColor (what is allowed)
        fPosition = (fPosition - fMin) / (fMax - fMin);
        return basegfx::interpolate(aCStart, aCEnd,
                                    nSteps > 1 ? floor(fPosition * nSteps) / double(nSteps - 1)
                                               : fPosition);
    }

    // check if we need to newly populate the needed interpolation data
    // or if we can re-use from last time.
    // If this scope is not entered, we do not need the binary search. It's
    // only a single buffered entry, and only used when more than three
    // ColorStops exist, but makes a huge difference compared with accessing
    // the sorted ColorStop vector each time.
    // NOTE: with this simple change I get very high hit rates, e.g. rotating
    //       a donut with gradient test '1' hit rate is at 0.99909440357755486
    if (rLastColorStopRange.mfOffsetStart == rLastColorStopRange.mfOffsetEnd
        || fPosition < rLastColorStopRange.mfOffsetStart
        || fPosition > rLastColorStopRange.mfOffsetEnd)
    {
        // access needed spot in sorted array using binary search
        // NOTE: This *seems* slow(er) when developing compared to just
        //       looping/accessing, but that's just due to the extensive
        //       debug test code created by the stl. In a pro version,
        //       all is good/fast as expected
        const auto upperBound(std::upper_bound(begin(), end(), BColorStop(fPosition),
                                               [](const BColorStop& x, const BColorStop& y) {
                                                   return x.getStopOffset() < y.getStopOffset();
                                               }));

        // no upper bound, done
        if (end() == upperBound)
            return back().getStopColor();

        // lower bound is one entry back, access that
        const auto lowerBound(upperBound - 1);

        // no lower bound, done
        if (end() == lowerBound)
            return back().getStopColor();

        // we have lower and upper bound, get colors and offsets
        rLastColorStopRange.maColorStart = lowerBound->getStopColor();
        rLastColorStopRange.maColorEnd = upperBound->getStopColor();
        rLastColorStopRange.mfOffsetStart = lowerBound->getStopOffset();
        rLastColorStopRange.mfOffsetEnd = upperBound->getStopOffset();
    }

    // when there are just two color steps this cannot happen, but when using
    // a range of colors this *may* be used inside the range to represent
    // single-colored regions inside a ColorRange. Use that color & done
    if (rLastColorStopRange.maColorStart == rLastColorStopRange.maColorEnd)
        return rLastColorStopRange.maColorStart;

    // calculate number of steps and adapted proportional
    // range for scaler in [0.0 .. 1.0]
    const double fAdaptedScaler(
        (fPosition - rLastColorStopRange.mfOffsetStart)
        / (rLastColorStopRange.mfOffsetEnd - rLastColorStopRange.mfOffsetStart));
    const sal_uInt32 nSteps(basegfx::utils::calculateNumberOfSteps(
        nRequestedSteps, rLastColorStopRange.maColorStart, rLastColorStopRange.maColorEnd));

    // interpolate & evtl. apply steps
    return interpolate(rLastColorStopRange.maColorStart, rLastColorStopRange.maColorEnd,
                       nSteps > 1 ? floor(fAdaptedScaler * nSteps) / double(nSteps - 1)
                                  : fAdaptedScaler);
}

/* Tooling method that allows to replace the StartColor in a
        vector of ColorStops. A vector in 'ordered state' is expected,
        so you may use/have used sortAndCorrect.
        This method is for convenience & backwards compatibility, please
        think about handling multi-colored gradients directly.
    */
void BColorStops::replaceStartColor(const BColor& rStart)
{
    BColorStops::iterator a1stNonStartColor(begin());

    // search for highest existing non-StartColor - CAUTION,
    // there might be none, one or multiple with StopOffset 0.0
    while (a1stNonStartColor != end() && a1stNonStartColor->getStopOffset() <= 0.0)
        a1stNonStartColor++;

    // create new ColorStops by 1st adding new one and then all
    // non-StartColor entries
    BColorStops aNewColorStops;

    aNewColorStops.reserve(size() + 1);
    aNewColorStops.emplace_back(0.0, rStart);
    aNewColorStops.insert(aNewColorStops.end(), a1stNonStartColor, end());

    // assign & done
    *this = aNewColorStops;
}

/* Tooling method that allows to replace the EndColor in a
        vector of ColorStops. A vector in 'ordered state' is expected,
        so you may use/have used sortAndCorrectColorStops.
        This method is for convenience & backwards compatibility, please
        think about handling multi-colored gradients directly.
    */
void BColorStops::replaceEndColor(const BColor& rEnd)
{
    // erase all evtl. existing EndColor(s)
    while (!empty() && basegfx::fTools::moreOrEqual(back().getStopOffset(), 1.0))
        pop_back();

    // add at the end of existing ColorStops
    emplace_back(1.0, rEnd);
}

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
void BColorStops::blendToIntensity(double fStartIntensity, double fEndIntensity,
                                   const BColor& rBlendColor)
{
    // no entries, done
    if (empty())
        return;

    // correct intensities (maybe assert when input was wrong)
    fStartIntensity = std::max(std::min(1.0, fStartIntensity), 0.0);
    fEndIntensity = std::max(std::min(1.0, fEndIntensity), 0.0);

    // all 100%, no real blend, done
    if (basegfx::fTools::equal(fStartIntensity, 1.0) && basegfx::fTools::equal(fEndIntensity, 1.0))
        return;

    // blend relative to StopOffset position
    for (auto& candidate : *this)
    {
        const double fOffset(candidate.getStopOffset());
        const double fIntensity((fStartIntensity * (1.0 - fOffset)) + (fEndIntensity * fOffset));
        candidate = basegfx::BColorStop(
            fOffset, basegfx::interpolate(rBlendColor, candidate.getStopColor(), fIntensity));
    }
}

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
void BColorStops::sortAndCorrect()
{
    // no content, we are done
    if (empty())
        return;

    if (1 == size())
    {
        // no gradient at all, but preserve given color
        // evtl. correct offset to be in valid range [0.0 .. 1.0]
        // NOTE: This does not move it to 0.0 or 1.0, it *can* still
        //       be somewhere in-between what is allowed
        const BColorStop aEntry(front());
        clear();
        emplace_back(std::max(0.0, std::min(1.0, aEntry.getStopOffset())), aEntry.getStopColor());

        // done
        return;
    }

    // start with sorting the input data. Remember that
    // this preserves the order of equal entries, where
    // equal is defined here by offset (see use operator==)
    std::sort(begin(), end());

    // prepare status values
    size_t write(0);

    // use the paradigm of a band machine with two heads, read
    // and write with write <= read all the time. Step over the
    // data using read and check for valid entry. If valid, decide
    // how to keep it
    for (size_t read(0); read < size(); read++)
    {
        // get offset of entry at read position
        double fOff((*this)[read].getStopOffset());

        if (basegfx::fTools::less(fOff, 0.0) && read + 1 < size())
        {
            // value < 0.0 and we have a next entry. check for gradient snippet
            // containing 0.0 resp. StartColor
            const double fOff2((*this)[read + 1].getStopOffset());

            if (basegfx::fTools::more(fOff2, 0.0))
            {
                // read is the start of a gradient snippet containing 0.0. Correct
                // entry to StartColor, interpolate to correct StartColor
                (*this)[read]
                    = BColorStop(0.0, basegfx::interpolate((*this)[read].getStopColor(),
                                                           (*this)[read + 1].getStopColor(),
                                                           (0.0 - fOff) / (fOff2 - fOff)));

                // adapt fOff
                fOff = 0.0;
            }
        }

        // step over < 0 values, these are outside and will be removed
        if (basegfx::fTools::less(fOff, 0.0))
        {
            continue;
        }

        if (basegfx::fTools::less(fOff, 1.0) && read + 1 < size())
        {
            // value < 1.0 and we have a next entry. check for gradient snippet
            // containing 1.0 resp. EndColor
            const double fOff2((*this)[read + 1].getStopOffset());

            if (basegfx::fTools::more(fOff2, 1.0))
            {
                // read is the start of a gradient snippet containing 1.0. Correct
                // next entry to EndColor, interpolate to correct EndColor
                (*this)[read + 1]
                    = BColorStop(1.0, basegfx::interpolate((*this)[read].getStopColor(),
                                                           (*this)[read + 1].getStopColor(),
                                                           (1.0 - fOff) / (fOff2 - fOff)));

                // adapt fOff
                fOff = 1.0;
            }
        }

        // step over > 1 values; even break, since all following
        // entries will also be bigger due to being sorted, so done
        if (basegfx::fTools::more(fOff, 1.0))
        {
            break;
        }

        // entry is valid value at read position
        // copy if write target is empty (write at start) or when
        // write target is different to read in color or offset
        if (0 == write || !((*this)[read] == (*this)[write - 1]))
        {
            if (write != read)
            {
                // copy read to write backwards to close gaps
                (*this)[write] = (*this)[read];
            }

            // always forward write position
            write++;
        }
    }

    // correct size when length is reduced. write is always at
    // last used position + 1
    if (size() > write)
    {
        if (0 == write)
        {
            // no valid entries at all, but not empty. This can only happen
            // when all entries are below 0.0 or above 1.0 (else a gradient
            // snippet spawning over both would have been detected)
            if (basegfx::fTools::less(back().getStopOffset(), 0.0))
            {
                // all outside too low, rescue last due to being closest to content
                const BColor aBackColor(back().getStopColor());
                clear();
                emplace_back(0.0, aBackColor);
            }
            else // if (basegfx::fTools::more(front().getStopOffset(), 1.0))
            {
                // all outside too high, rescue first due to being closest to content
                const BColor aFrontColor(front().getStopColor());
                clear();
                emplace_back(1.0, aFrontColor);
            }
        }
        else
        {
            resize(write);
        }
    }
}

bool BColorStops::checkPenultimate() const
{
    // not needed when no ColorStops
    if (empty())
        return false;

    // not needed when last ColorStop at the end or outside
    if (basegfx::fTools::moreOrEqual(back().getStopOffset(), 1.0))
        return false;

    // get penultimate entry
    const auto penultimate(rbegin() + 1);

    // if there is none, we need no correction and are done
    if (penultimate == rend())
        return false;

    // not needed when the last two ColorStops have different offset, then
    // a visible range will be processed already
    if (!basegfx::fTools::equal(back().getStopOffset(), penultimate->getStopOffset()))
        return false;

    // not needed when the last two ColorStops have the same Color, then the
    // range before solves the problem
    if (back().getStopColor() == penultimate->getStopColor())
        return false;

    return true;
}

/* Tooling method to check if a ColorStop vector is defined
        by a single color. It returns true if this is the case.
        If true is returned, rSingleColor contains that single
        color for convenience.
        NOTE: If no ColorStop is defined, a fallback to BColor-default
                (which is black) and true will be returned
    */
bool BColorStops::isSingleColor(BColor& rSingleColor) const
{
    if (empty())
    {
        rSingleColor = BColor();
        return true;
    }

    if (1 == size())
    {
        rSingleColor = front().getStopColor();
        return true;
    }

    rSingleColor = front().getStopColor();

    for (auto const& rCandidate : *this)
    {
        if (rCandidate.getStopColor() != rSingleColor)
            return false;
    }

    return true;
}

/* Tooling method to reverse ColorStops, including offsets.
        When also mirroring offsets a valid sort keeps valid.
    */
void BColorStops::reverseColorStops()
{
    // can use std::reverse, but also need to adapt offset(s)
    std::reverse(begin(), end());
    for (auto& candidate : *this)
        candidate = BColorStop(1.0 - candidate.getStopOffset(), candidate.getStopColor());
}

// createSpaceAtStart creates fOffset space at start by
// translating/scaling all entries to the right
void BColorStops::createSpaceAtStart(double fOffset)
{
    // nothing to do if empty
    if (empty())
        return;

    // correct offset to [0.0 .. 1.0]
    fOffset = std::max(std::min(1.0, fOffset), 0.0);

    // nothing to do if 0.0 == offset
    if (basegfx::fTools::equalZero(fOffset))
        return;

    BColorStops aNewStops;

    for (const auto& candidate : *this)
    {
        aNewStops.emplace_back(fOffset + (candidate.getStopOffset() * (1.0 - fOffset)),
                               candidate.getStopColor());
    }

    *this = aNewStops;
}

// removeSpaceAtStart removes fOffset space from start by
// translating/scaling entries more or equal to fOffset
// to the left. Entries less than fOffset will be removed
void BColorStops::removeSpaceAtStart(double fOffset)
{
    // nothing to do if empty
    if (empty())
        return;

    // correct factor to [0.0 .. 1.0]
    fOffset = std::max(std::min(1.0, fOffset), 0.0);

    // nothing to do if fOffset == 0.0
    if (basegfx::fTools::equalZero(fOffset))
        return;

    BColorStops aNewStops;
    const double fMul(basegfx::fTools::equal(fOffset, 1.0) ? 1.0 : 1.0 / (1.0 - fOffset));

    for (const auto& candidate : *this)
    {
        if (basegfx::fTools::moreOrEqual(candidate.getStopOffset(), fOffset))
        {
            aNewStops.emplace_back((candidate.getStopOffset() - fOffset) * fMul,
                                   candidate.getStopColor());
        }
    }

    *this = aNewStops;
}

// try to detect if an empty/no-color-change area exists
// at the start and return offset to it. Returns 0.0 if not.
double BColorStops::detectPossibleOffsetAtStart() const
{
    BColor aSingleColor;
    const bool bSingleColor(isSingleColor(aSingleColor));

    // no useful offset for single color
    if (bSingleColor)
        return 0.0;

    // here we know that we have at least two colors, so we have a
    // color change. Find colors left and right of that first color change
    BColorStops::const_iterator aColorR(begin());
    BColorStops::const_iterator aColorL(aColorR++);

    // aColorR would 1st get equal to end(), so no need to also check aColorL
    // for end(). Loop as long as same color. Since we *have* a color change
    // not even aColorR can get equal to end() before color inequality, but
    // keep for safety
    while (aColorR != end() && aColorL->getStopColor() == aColorR->getStopColor())
    {
        aColorL++;
        aColorR++;
    }

    // also for safety: access values at aColorL below *only*
    // if not equal to end(), but can theoretically not happen
    if (aColorL == end())
    {
        return 0.0;
    }

    // return offset (maybe 0.0 what is OK)
    return aColorL->getStopOffset();
}

//  checks whether the color stops are symmetrical in color and offset.
bool BColorStops::isSymmetrical() const
{
    if (empty())
        return false;
    if (1 == size())
        return basegfx::fTools::equal(0.5, front().getStopOffset());

    BColorStops::const_iterator aIter(begin()); // for going forward
    BColorStops::const_iterator aRIter(end()); // for going backward
    --aRIter;
    // We have at least two elements, so aIter <= aRIter fails before iterators no longer point to
    // an element.
    while (aIter <= aRIter && aIter->getStopColor().equal(aRIter->getStopColor())
           && basegfx::fTools::equal(aIter->getStopOffset(), 1.0 - aRIter->getStopOffset()))
    {
        ++aIter;
        --aRIter;
    }
    return aIter > aRIter;
}

void BColorStops::doApplyAxial()
{
    // prepare new ColorStops
    basegfx::BColorStops aNewColorStops;

    // add gradient stops in reverse order, scaled to [0.0 .. 0.5]
    basegfx::BColorStops::const_reverse_iterator aRevCurrColor(rbegin());

    while (aRevCurrColor != rend())
    {
        aNewColorStops.emplace_back((1.0 - aRevCurrColor->getStopOffset()) * 0.5,
                                    aRevCurrColor->getStopColor());
        aRevCurrColor++;
    }

    // prepare non-reverse run
    basegfx::BColorStops::const_iterator aCurrColor(begin());

    if (basegfx::fTools::equalZero(aCurrColor->getStopOffset()))
    {
        // Caution: do not add 1st entry again, that would be double since it was
        // already added as last element of the inverse run above. But only if
        // the gradient has a start entry for 0.0 aka StartColor, else it is correct.
        aCurrColor++;
    }

    // add gradient stops in non-reverse order, translated and scaled to [0.5 .. 1.0]
    while (aCurrColor != end())
    {
        aNewColorStops.emplace_back((aCurrColor->getStopOffset() * 0.5) + 0.5,
                                    aCurrColor->getStopColor());
        aCurrColor++;
    }

    // apply color stops
    *this = aNewColorStops;
}

void BColorStops::doApplySteps(sal_uInt16 nStepCount)
{
    // check for zero or invalid steps setting -> done
    if (0 == nStepCount || nStepCount > 100)
        return;

    // no change needed if single color
    BColor aSingleColor;
    if (isSingleColor(aSingleColor))
        return;

    // prepare new color stops, get L/R iterators for segments
    basegfx::BColorStops aNewColorStops;
    basegfx::BColorStops::const_iterator aColorR(begin());
    basegfx::BColorStops::const_iterator aColorL(aColorR++);

    while (aColorR != end())
    {
        // get start/end color for segment
        const double fStart(aColorL->getStopOffset());
        const double fDelta(aColorR->getStopOffset() - fStart);

        if (aNewColorStops.empty() || aNewColorStops.back() != *aColorL)
        {
            // add start color, but check if it is already there - which is the
            // case from the 2nd segment on due to a new segment starting with
            // the same color as the previous one ended
            aNewColorStops.push_back(*aColorL);
        }
        if (!basegfx::fTools::equalZero(fDelta))
        {
            // create in-between steps, always two at the same position to
            // define a 'hard' color stop. Get start/end color for the segment
            const basegfx::BColor& rStartColor(aColorL->getStopColor());
            const basegfx::BColor& rEndColor(aColorR->getStopColor());

            if (rStartColor != rEndColor)
            {
                // get relative single-step width
                // tdf155852 Use same method for the color as in rendering.
                const double fSingleStep(1.0 / static_cast<double>(nStepCount - 1));
                const double fOffsetStep(fDelta / static_cast<double>(nStepCount));

                for (sal_uInt16 a(1); a < nStepCount; a++)
                {
                    // calculate stop position since being used twice
                    const double fPosition(fStart + fOffsetStep * static_cast<double>(a));

                    // add end color of previous sub-segment
                    aNewColorStops.emplace_back(
                        fPosition, basegfx::interpolate(rStartColor, rEndColor,
                                                        static_cast<double>(a - 1) * fSingleStep));

                    // add start color of current sub-segment
                    aNewColorStops.emplace_back(
                        fPosition, basegfx::interpolate(rStartColor, rEndColor,
                                                        static_cast<double>(a) * fSingleStep));
                }
            }
        }

        // always add end color of segment
        aNewColorStops.push_back(*aColorR);

        // next segment
        aColorL++;
        aColorR++;
    }

    // apply the change to color stops
    *this = aNewColorStops;
}

std::string BGradient::GradientStyleToString(css::awt::GradientStyle eStyle)
{
    switch (eStyle)
    {
        case css::awt::GradientStyle::GradientStyle_LINEAR:
            return "LINEAR";

        case css::awt::GradientStyle::GradientStyle_AXIAL:
            return "AXIAL";

        case css::awt::GradientStyle::GradientStyle_RADIAL:
            return "RADIAL";

        case css::awt::GradientStyle::GradientStyle_ELLIPTICAL:
            return "ELLIPTICAL";

        case css::awt::GradientStyle::GradientStyle_SQUARE:
            return "SQUARE";

        case css::awt::GradientStyle::GradientStyle_RECT:
            return "RECT";

        case css::awt::GradientStyle::GradientStyle_MAKE_FIXED_SIZE:
            return "MAKE_FIXED_SIZE";
    }

    return "";
}

BGradient BGradient::fromJSON(std::u16string_view rJSON)
{
    StringMap aMap(lcl_jsonToStringMap(rJSON));
    return lcl_buildGradientFromStringMap(aMap);
}

BGradient::BGradient()
    : eStyle(css::awt::GradientStyle_LINEAR)
    , aColorStops()
    , nAngle(0)
    , nBorder(0)
    , nOfsX(50)
    , nOfsY(50)
    , nIntensStart(100)
    , nIntensEnd(100)
    , nStepCount(0)
{
    aColorStops.emplace_back(0.0, BColor(0.0, 0.0, 0.0)); // COL_BLACK
    aColorStops.emplace_back(1.0, BColor(1.0, 1.0, 1.0)); // COL_WHITE
}

BGradient::BGradient(const basegfx::BColorStops& rColorStops, css::awt::GradientStyle eTheStyle,
                     Degree10 nTheAngle, sal_uInt16 nXOfs, sal_uInt16 nYOfs, sal_uInt16 nTheBorder,
                     sal_uInt16 nStartIntens, sal_uInt16 nEndIntens, sal_uInt16 nSteps)
    : eStyle(eTheStyle)
    , aColorStops(rColorStops)
    , nAngle(nTheAngle)
    , nBorder(nTheBorder)
    , nOfsX(nXOfs)
    , nOfsY(nYOfs)
    , nIntensStart(nStartIntens)
    , nIntensEnd(nEndIntens)
    , nStepCount(nSteps)
{
    SetColorStops(aColorStops);
}

bool BGradient::operator==(const BGradient& rGradient) const
{
    return (eStyle == rGradient.eStyle && aColorStops == rGradient.aColorStops
            && nAngle == rGradient.nAngle && nBorder == rGradient.nBorder
            && nOfsX == rGradient.nOfsX && nOfsY == rGradient.nOfsY
            && nIntensStart == rGradient.nIntensStart && nIntensEnd == rGradient.nIntensEnd
            && nStepCount == rGradient.nStepCount);
}

void BGradient::SetColorStops(const basegfx::BColorStops& rSteps)
{
    aColorStops = rSteps;
    aColorStops.sortAndCorrect();
    if (aColorStops.empty())
        aColorStops.emplace_back(0.0, basegfx::BColor());
}

namespace
{
OUString AsRGBHexString(const ColorToBColorConverter& rVal)
{
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(6) << sal_uInt32(rVal);
    return OUString::createFromAscii(ss.str());
}
}

boost::property_tree::ptree BGradient::dumpAsJSON() const
{
    boost::property_tree::ptree aTree;

    aTree.put("style", BGradient::GradientStyleToString(eStyle));
    const ColorToBColorConverter aStart(GetColorStops().front().getStopColor());
    aTree.put("startcolor", AsRGBHexString(aStart.GetRGBColor()));
    const ColorToBColorConverter aEnd(GetColorStops().back().getStopColor());
    aTree.put("endcolor", AsRGBHexString(aEnd.GetRGBColor()));
    aTree.put("angle", std::to_string(nAngle.get()));
    aTree.put("border", std::to_string(nBorder));
    aTree.put("x", std::to_string(nOfsX));
    aTree.put("y", std::to_string(nOfsY));
    aTree.put("intensstart", std::to_string(nIntensStart));
    aTree.put("intensend", std::to_string(nIntensEnd));
    aTree.put("stepcount", std::to_string(nStepCount));

    return aTree;
}

void BGradient::tryToRecreateBorder(basegfx::BColorStops* pAssociatedTransparencyStops)
{
    // border already set, do not try to recreate
    if (0 != GetBorder())
        return;

    BColor aSingleColor;
    const bool bSingleColor(GetColorStops().isSingleColor(aSingleColor));

    // no need to recreate with single color
    if (bSingleColor)
        return;

    const bool bIsAxial(css::awt::GradientStyle_AXIAL == GetGradientStyle());

    if (bIsAxial)
    {
        // for axial due to reverse used gradient work reversed
        aColorStops.reverseColorStops();
        if (nullptr != pAssociatedTransparencyStops)
            pAssociatedTransparencyStops->reverseColorStops();
    }

    // check if we have space at start of range [0.0 .. 1.0] that
    // may be interpreted as 'border' -> same color. That may involve
    // different scenarios, e.g. 1st index > 0.0, but also a non-zero
    // number of same color entries, or a combination of both
    const double fOffset(aColorStops.detectPossibleOffsetAtStart());

    if (!basegfx::fTools::equalZero(fOffset))
    {
        // we have a border area, indeed re-create
        aColorStops.removeSpaceAtStart(fOffset);
        if (nullptr != pAssociatedTransparencyStops)
            pAssociatedTransparencyStops->removeSpaceAtStart(fOffset);

        // ...and create border value
        SetBorder(static_cast<sal_uInt16>(std::lround(fOffset * 100.0)));
    }

    if (bIsAxial)
    {
        // take back reverse
        aColorStops.reverseColorStops();
        if (nullptr != pAssociatedTransparencyStops)
            pAssociatedTransparencyStops->reverseColorStops();
    }
}

void BGradient::tryToApplyBorder()
{
    // no border to apply, done
    if (0 == GetBorder())
        return;

    // NOTE: no new start node is added. The new ColorStop
    //       mechanism does not need entries at 0.0 and 1.0.
    //       In case this is needed, do that in the caller
    const double fOffset(GetBorder() * 0.01);

    if (css::awt::GradientStyle_AXIAL == GetGradientStyle())
    {
        // for axial due to reverse used gradient work reversed
        aColorStops.reverseColorStops();
        aColorStops.createSpaceAtStart(fOffset);
        aColorStops.reverseColorStops();
    }
    else
    {
        // apply border to GradientSteps
        aColorStops.createSpaceAtStart(fOffset);
    }

    // set changed values
    SetBorder(0);
}

void BGradient::tryToApplyStartEndIntensity()
{
    // already on default, nothing to apply
    if (100 == GetStartIntens() && 100 == GetEndIntens())
        return;

    // apply 'old' blend stuff, blend against black
    aColorStops.blendToIntensity(GetStartIntens() * 0.01, GetEndIntens() * 0.01,
                                 BColor()); // COL_BLACK

    // set values to default
    SetStartIntens(100);
    SetEndIntens(100);
}

void BGradient::tryToConvertToAxial()
{
    if (css::awt::GradientStyle_LINEAR != GetGradientStyle() || 0 != GetBorder()
        || GetColorStops().empty())
        return;

    if (!GetColorStops().isSymmetrical())
        return;

    SetGradientStyle(css::awt::GradientStyle_AXIAL);

    // Stretch the first half of the color stops to double width
    // and collect them in a new color stops vector.
    BColorStops aAxialColorStops;
    aAxialColorStops.reserve(std::ceil(GetColorStops().size() / 2.0));
    BColorStops::const_iterator aIter(GetColorStops().begin());
    while (basegfx::fTools::lessOrEqual(aIter->getStopOffset(), 0.5))
    {
        BColorStop aNextStop(std::clamp((*aIter).getStopOffset() * 2.0, 0.0, 1.0),
                             (*aIter).getStopColor());
        aAxialColorStops.push_back(aNextStop);
        ++aIter;
    }
    // Axial gradients have outmost color as last color stop.
    aAxialColorStops.reverseColorStops();

    SetColorStops(aAxialColorStops);
}

void BGradient::tryToApplyAxial()
{
    // only need to do something if css::awt::GradientStyle_AXIAL, else done
    if (GetGradientStyle() != css::awt::GradientStyle_AXIAL)
        return;

    // apply the change to color stops
    aColorStops.doApplyAxial();

    // set style to GradientStyle_LINEAR
    SetGradientStyle(css::awt::GradientStyle_LINEAR);
}

void BGradient::tryToApplySteps()
{
    // check for zero or invalid steps setting -> done
    if (0 == GetSteps() || GetSteps() > 100)
        return;

    // do the action
    aColorStops.doApplySteps(GetSteps());

    // set value to default
    SetSteps(0);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
