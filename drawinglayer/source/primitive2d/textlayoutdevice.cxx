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

#include <sal/config.h>

#include <algorithm>

#include <com/sun/star/uno/XComponentContext.hpp>

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/attribute/fontattribute.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/unique_disposing_ptr.hxx>
#include <osl/diagnose.h>
#include <tools/gen.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/kernarray.hxx>
#include <vcl/timer.hxx>
#include <vcl/virdev.hxx>
#include <vcl/font.hxx>
#include <vcl/metric.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <vcl/svapp.hxx>

namespace drawinglayer::primitive2d
{
namespace
{
class ImpTimedRefDev;

// VDev RevDevice provider

//the scoped_timed_RefDev owns an ImpTimeRefDev and releases it on dtor
//or disposing of the default XComponentContext which causes the underlying
//OutputDevice to get released

//The ImpTimerRefDev itself, if the timeout ever gets hit, will call
//reset on the scoped_timed_RefDev to release the ImpTimerRefDev early
//if it's unused for a few minutes
class scoped_timed_RefDev : public comphelper::unique_disposing_ptr<ImpTimedRefDev>
{
public:
    scoped_timed_RefDev()
        : comphelper::unique_disposing_ptr<ImpTimedRefDev>(
              (css::uno::Reference<css::lang::XComponent>(
                  ::comphelper::getProcessComponentContext(), css::uno::UNO_QUERY_THROW)))
    {
    }
};

class the_scoped_timed_RefDev : public rtl::Static<scoped_timed_RefDev, the_scoped_timed_RefDev>
{
};

class ImpTimedRefDev : public Timer
{
    scoped_timed_RefDev& mrOwnerOfMe;
    VclPtr<VirtualDevice> mpVirDev;
    sal_uInt32 mnUseCount;

public:
    explicit ImpTimedRefDev(scoped_timed_RefDev& rOwnerofMe);
    virtual ~ImpTimedRefDev() override;
    virtual void Invoke() override;

    VirtualDevice& acquireVirtualDevice();
    void releaseVirtualDevice();
};

ImpTimedRefDev::ImpTimedRefDev(scoped_timed_RefDev& rOwnerOfMe)
    : Timer("drawinglayer ImpTimedRefDev destroy mpVirDev")
    , mrOwnerOfMe(rOwnerOfMe)
    , mpVirDev(nullptr)
    , mnUseCount(0)
{
    SetTimeout(3L * 60L * 1000L); // three minutes
    Start();
}

ImpTimedRefDev::~ImpTimedRefDev()
{
    OSL_ENSURE(0 == mnUseCount, "destruction of a still used ImpTimedRefDev (!)");
    const SolarMutexGuard aSolarGuard;
    mpVirDev.disposeAndClear();
}

void ImpTimedRefDev::Invoke()
{
    // for obvious reasons, do not call anything after this
    mrOwnerOfMe.reset();
}

VirtualDevice& ImpTimedRefDev::acquireVirtualDevice()
{
    if (!mpVirDev)
    {
        mpVirDev = VclPtr<VirtualDevice>::Create();
        mpVirDev->SetReferenceDevice(VirtualDevice::RefDevMode::MSO1);
    }

    if (!mnUseCount)
    {
        Stop();
    }

    mnUseCount++;

    return *mpVirDev;
}

void ImpTimedRefDev::releaseVirtualDevice()
{
    OSL_ENSURE(mnUseCount, "mismatch call number to releaseVirtualDevice() (!)");
    mnUseCount--;

    if (!mnUseCount)
    {
        Start();
    }
}

VirtualDevice& acquireGlobalVirtualDevice()
{
    scoped_timed_RefDev& rStdRefDevice = the_scoped_timed_RefDev::get();

    if (!rStdRefDevice)
        rStdRefDevice.reset(new ImpTimedRefDev(rStdRefDevice));

    return rStdRefDevice->acquireVirtualDevice();
}

void releaseGlobalVirtualDevice()
{
    scoped_timed_RefDev& rStdRefDevice = the_scoped_timed_RefDev::get();

    OSL_ENSURE(rStdRefDevice,
               "releaseGlobalVirtualDevice() without prior acquireGlobalVirtualDevice() call(!)");
    rStdRefDevice->releaseVirtualDevice();
}

} // end of anonymous namespace

TextLayouterDevice::TextLayouterDevice()
    : mrDevice(acquireGlobalVirtualDevice())
{
}

TextLayouterDevice::~TextLayouterDevice() COVERITY_NOEXCEPT_FALSE { releaseGlobalVirtualDevice(); }

void TextLayouterDevice::setFont(const vcl::Font& rFont)
{
    mrDevice.SetFont(rFont);
    mnFontScalingFixX = 1.0;
    mnFontScalingFixY = 1.0;
}

void TextLayouterDevice::setFontAttribute(const attribute::FontAttribute& rFontAttribute,
                                          double fFontScaleX, double fFontScaleY,
                                          const css::lang::Locale& rLocale)
{
    vcl::Font aFont
        = getVclFontFromFontAttribute(rFontAttribute, fFontScaleX, fFontScaleY, 0.0, rLocale);
    setFont(aFont);
    Size aFontSize = aFont.GetFontSize();
    mnFontScalingFixY = fFontScaleY / aFontSize.Height();
    mnFontScalingFixX = fFontScaleX / (aFontSize.Width() ? aFontSize.Width() : aFontSize.Height());
}

double TextLayouterDevice::getOverlineOffset() const
{
    const ::FontMetric& rMetric = mrDevice.GetFontMetric();
    double fRet = (rMetric.GetInternalLeading() / 2.0) - rMetric.GetAscent();
    return fRet * mnFontScalingFixY;
}

double TextLayouterDevice::getUnderlineOffset() const
{
    const ::FontMetric& rMetric = mrDevice.GetFontMetric();
    double fRet = rMetric.GetDescent() / 2.0;
    return fRet * mnFontScalingFixY;
}

double TextLayouterDevice::getStrikeoutOffset() const
{
    const ::FontMetric& rMetric = mrDevice.GetFontMetric();
    double fRet = (rMetric.GetAscent() - rMetric.GetInternalLeading()) / 3.0;
    return fRet * mnFontScalingFixY;
}

double TextLayouterDevice::getOverlineHeight() const
{
    const ::FontMetric& rMetric = mrDevice.GetFontMetric();
    double fRet = rMetric.GetInternalLeading() / 2.5;
    return fRet * mnFontScalingFixY;
}

double TextLayouterDevice::getUnderlineHeight() const
{
    const ::FontMetric& rMetric = mrDevice.GetFontMetric();
    double fRet = rMetric.GetDescent() / 4.0;
    return fRet * mnFontScalingFixY;
}

double TextLayouterDevice::getTextHeight() const
{
    return mrDevice.GetTextHeightDouble() * mnFontScalingFixY;
}

double TextLayouterDevice::getTextWidth(const OUString& rText, sal_uInt32 nIndex,
                                        sal_uInt32 nLength) const
{
    return mrDevice.GetTextWidthDouble(rText, nIndex, nLength) * mnFontScalingFixX;
}

void TextLayouterDevice::getTextOutlines(basegfx::B2DPolyPolygonVector& rB2DPolyPolyVector,
                                         const OUString& rText, sal_uInt32 nIndex,
                                         sal_uInt32 nLength, const std::vector<double>& rDXArray,
                                         const std::vector<sal_Bool>& rKashidaArray) const
{
    const sal_uInt32 nDXArrayCount(rDXArray.size());
    sal_uInt32 nTextLength(nLength);
    const sal_uInt32 nStringLength(rText.getLength());

    if (nTextLength + nIndex > nStringLength)
    {
        nTextLength = nStringLength - nIndex;
    }

    if (nDXArrayCount)
    {
        OSL_ENSURE(nDXArrayCount == nTextLength,
                   "DXArray size does not correspond to text portion size (!)");

        KernArray aIntegerDXArray;
        aIntegerDXArray.reserve(nDXArrayCount);
        for (sal_uInt32 a(0); a < nDXArrayCount; a++)
            aIntegerDXArray.push_back(basegfx::fround(rDXArray[a]));

        mrDevice.GetTextOutlines(rB2DPolyPolyVector, rText, nIndex, nIndex, nLength, 0,
                                 aIntegerDXArray, rKashidaArray);
    }
    else
    {
        mrDevice.GetTextOutlines(rB2DPolyPolyVector, rText, nIndex, nIndex, nLength);
    }
    if (!rtl_math_approxEqual(mnFontScalingFixY, 1.0)
        || !rtl_math_approxEqual(mnFontScalingFixX, 1.0))
    {
        auto scale = basegfx::utils::createScaleB2DHomMatrix(mnFontScalingFixX, mnFontScalingFixY);
        for (auto& poly : rB2DPolyPolyVector)
            poly.transform(scale);
    }
}

basegfx::B2DRange TextLayouterDevice::getTextBoundRect(const OUString& rText, sal_uInt32 nIndex,
                                                       sal_uInt32 nLength) const
{
    sal_uInt32 nTextLength(nLength);
    const sal_uInt32 nStringLength(rText.getLength());

    if (nTextLength + nIndex > nStringLength)
    {
        nTextLength = nStringLength - nIndex;
    }

    if (nTextLength)
    {
        basegfx::B2DRange aRect;
        mrDevice.GetTextBoundRect(aRect, rText, nIndex, nIndex, nLength);
        if (!rtl_math_approxEqual(mnFontScalingFixY, 1.0)
            || !rtl_math_approxEqual(mnFontScalingFixX, 1.0))
        {
            aRect.transform(
                basegfx::utils::createScaleB2DHomMatrix(mnFontScalingFixX, mnFontScalingFixY));
        }
        return aRect;
    }

    return basegfx::B2DRange();
}

double TextLayouterDevice::getFontAscent() const
{
    const ::FontMetric& rMetric = mrDevice.GetFontMetric();
    return rMetric.GetAscent() * mnFontScalingFixY;
}

double TextLayouterDevice::getFontDescent() const
{
    const ::FontMetric& rMetric = mrDevice.GetFontMetric();
    return rMetric.GetDescent() * mnFontScalingFixY;
}

void TextLayouterDevice::addTextRectActions(const ::tools::Rectangle& rRectangle,
                                            const OUString& rText, DrawTextFlags nStyle,
                                            GDIMetaFile& rGDIMetaFile) const
{
    mrDevice.AddTextRectActions(rRectangle, rText, nStyle, rGDIMetaFile);
}

std::vector<double> TextLayouterDevice::getTextArray(const OUString& rText, sal_uInt32 nIndex,
                                                     sal_uInt32 nLength, bool bCaret) const
{
    std::vector<double> aRetval;
    sal_uInt32 nTextLength(nLength);
    const sal_uInt32 nStringLength(rText.getLength());

    if (nTextLength + nIndex > nStringLength)
    {
        nTextLength = nStringLength - nIndex;
    }

    if (nTextLength)
    {
        KernArray aArray;
        mrDevice.GetTextArray(rText, &aArray, nIndex, nTextLength, bCaret);
        aRetval.reserve(aArray.size());
        for (size_t i = 0, nEnd = aArray.size(); i < nEnd; ++i)
            aRetval.push_back(aArray[i] * mnFontScalingFixX);
    }

    return aRetval;
}

// helper methods for vcl font handling

vcl::Font getVclFontFromFontAttribute(const attribute::FontAttribute& rFontAttribute,
                                      double fFontScaleX, double fFontScaleY, double fFontRotation,
                                      const css::lang::Locale& rLocale)
{
    // detect FontScaling
    const sal_uInt32 nHeight(basegfx::fround(fabs(fFontScaleY)));
    const sal_uInt32 nWidth(basegfx::fround(fabs(fFontScaleX)));
    const bool bFontIsScaled(nHeight != nWidth);

#ifdef _WIN32
    // for WIN32 systems, start with creating an unscaled font. If FontScaling
    // is wanted, that width needs to be adapted using FontMetric again to get a
    // width of the unscaled font
    vcl::Font aRetval(rFontAttribute.getFamilyName(), rFontAttribute.getStyleName(),
                      Size(0, nHeight));
#else
    // for non-WIN32 systems things are easier since these accept a Font creation
    // with initially nWidth != nHeight for FontScaling. Despite that, use zero for
    // FontWidth when no scaling is used to explicitly have that zero when e.g. the
    // Font would be recorded in a MetaFile (The MetaFile FontAction WILL record a
    // set FontWidth; import that in a WIN32 system, and trouble is there)
    vcl::Font aRetval(rFontAttribute.getFamilyName(), rFontAttribute.getStyleName(),
                      Size(bFontIsScaled ? std::max<sal_uInt32>(nWidth, 1) : 0, nHeight));
#endif
    // define various other FontAttribute
    aRetval.SetAlignment(ALIGN_BASELINE);
    aRetval.SetCharSet(rFontAttribute.getSymbol() ? RTL_TEXTENCODING_SYMBOL
                                                  : RTL_TEXTENCODING_UNICODE);
    aRetval.SetVertical(rFontAttribute.getVertical());
    aRetval.SetWeight(static_cast<FontWeight>(rFontAttribute.getWeight()));
    aRetval.SetItalic(rFontAttribute.getItalic() ? ITALIC_NORMAL : ITALIC_NONE);
    aRetval.SetOutline(rFontAttribute.getOutline());
    aRetval.SetPitch(rFontAttribute.getMonospaced() ? PITCH_FIXED : PITCH_VARIABLE);
    aRetval.SetLanguage(LanguageTag::convertToLanguageType(rLocale, false));

#ifdef _WIN32
    // for WIN32 systems, correct the FontWidth if FontScaling is used
    if (bFontIsScaled && nHeight > 0)
    {
        const FontMetric aUnscaledFontMetric(
            Application::GetDefaultDevice()->GetFontMetric(aRetval));

        if (aUnscaledFontMetric.GetAverageFontWidth() > 0)
        {
            const double fScaleFactor(static_cast<double>(nWidth) / static_cast<double>(nHeight));
            const sal_uInt32 nScaledWidth(basegfx::fround(
                static_cast<double>(aUnscaledFontMetric.GetAverageFontWidth()) * fScaleFactor));
            aRetval.SetAverageFontWidth(nScaledWidth);
        }
    }
#endif
    // handle FontRotation (if defined)
    if (!basegfx::fTools::equalZero(fFontRotation))
    {
        int aRotate10th(-basegfx::rad2deg<10>(fFontRotation));
        aRetval.SetOrientation(Degree10(aRotate10th % 3600));
    }

    return aRetval;
}

attribute::FontAttribute getFontAttributeFromVclFont(basegfx::B2DVector& o_rSize,
                                                     const vcl::Font& rFont, bool bRTL,
                                                     bool bBiDiStrong)
{
    const attribute::FontAttribute aRetval(
        rFont.GetFamilyName(), rFont.GetStyleName(), static_cast<sal_uInt16>(rFont.GetWeight()),
        RTL_TEXTENCODING_SYMBOL == rFont.GetCharSet(), rFont.IsVertical(),
        ITALIC_NONE != rFont.GetItalic(), PITCH_FIXED == rFont.GetPitch(), rFont.IsOutline(), bRTL,
        bBiDiStrong);
    // TODO: eKerning

    // set FontHeight and init to no FontScaling
    o_rSize.setY(std::max<tools::Long>(rFont.GetFontSize().getHeight(), 0));
    o_rSize.setX(o_rSize.getY());

#ifdef _WIN32
    // for WIN32 systems, the FontScaling at the Font is detected by
    // checking that FontWidth != 0. When FontScaling is used, WIN32
    // needs to do extra stuff to detect the correct width (since it's
    // zero and not equal the font height) and its relationship to
    // the height
    if (rFont.GetFontSize().getWidth() > 0)
    {
        vcl::Font aUnscaledFont(rFont);
        aUnscaledFont.SetAverageFontWidth(0);
        const FontMetric aUnscaledFontMetric(
            Application::GetDefaultDevice()->GetFontMetric(aUnscaledFont));

        if (aUnscaledFontMetric.GetAverageFontWidth() > 0)
        {
            const double fScaleFactor(
                static_cast<double>(rFont.GetFontSize().getWidth())
                / static_cast<double>(aUnscaledFontMetric.GetAverageFontWidth()));
            o_rSize.setX(fScaleFactor * o_rSize.getY());
        }
    }
#else
    // For non-WIN32 systems the detection is the same, but the value
    // is easier achieved since width == height is interpreted as no
    // scaling. Ergo, Width == 0 means width == height, and width != 0
    // means the scaling is in the direct relation of width to height
    if (rFont.GetFontSize().getWidth() > 0)
    {
        o_rSize.setX(static_cast<double>(rFont.GetFontSize().getWidth()));
    }
#endif
    return aRetval;
}

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
