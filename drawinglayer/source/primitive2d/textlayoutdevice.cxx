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

#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/scoped_disposing_ptr.hxx>
#include <vcl/timer.hxx>
#include <vcl/virdev.hxx>
#include <vcl/font.hxx>
#include <vcl/metric.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <vcl/svapp.hxx>

//////////////////////////////////////////////////////////////////////////////
// VDev RevDevice provider

namespace
{
    class ImpTimedRefDev;

    //the scoped_timed_RefDev owns a ImpTimeRefDev and releases it on dtor
    //or disposing of the default XComponentContext which causes the underlying
    //OutputDevice to get released
    //
    //The ImpTimerRefDev itself, if the timeout ever gets hit, will call
    //reset on the scoped_timed_RefDev to release the ImpTimerRefDev early
    //if its unused for a few minutes
    class scoped_timed_RefDev : public comphelper::scoped_disposing_ptr<ImpTimedRefDev>
    {
    public:
        scoped_timed_RefDev() : comphelper::scoped_disposing_ptr<ImpTimedRefDev>((::com::sun::star::uno::Reference<com::sun::star::lang::XComponent>(::comphelper::getProcessComponentContext(), ::com::sun::star::uno::UNO_QUERY_THROW)))
        {
        }
    };

    class the_scoped_timed_RefDev : public rtl::Static<scoped_timed_RefDev, the_scoped_timed_RefDev> {};

    class ImpTimedRefDev : public Timer
    {
        scoped_timed_RefDev&                mrOwnerOfMe;
        VirtualDevice*                      mpVirDev;
        sal_uInt32                          mnUseCount;

    public:
        explicit ImpTimedRefDev(scoped_timed_RefDev& rOwnerofMe);
        ~ImpTimedRefDev();
        virtual void Timeout();

        VirtualDevice& acquireVirtualDevice();
        void releaseVirtualDevice();
    };

    ImpTimedRefDev::ImpTimedRefDev(scoped_timed_RefDev& rOwnerOfMe)
    :   mrOwnerOfMe(rOwnerOfMe),
        mpVirDev(0L),
        mnUseCount(0L)
    {
        SetTimeout(3L * 60L * 1000L); // three minutes
        Start();
    }

    ImpTimedRefDev::~ImpTimedRefDev()
    {
        OSL_ENSURE(0L == mnUseCount, "destruction of a still used ImpTimedRefDev (!)");
        const SolarMutexGuard aGuard;
        delete mpVirDev;
    }

    void ImpTimedRefDev::Timeout()
    {
        // for obvious reasons, do not call anything after this
        mrOwnerOfMe.reset();
    }

    VirtualDevice& ImpTimedRefDev::acquireVirtualDevice()
    {
        if(!mpVirDev)
        {
            mpVirDev = new VirtualDevice();
            mpVirDev->SetReferenceDevice( VirtualDevice::REFDEV_MODE_MSO1 );
        }

        if(!mnUseCount)
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

        if(!mnUseCount)
        {
            Start();
        }
    }
} // end of anonymous namespace

//////////////////////////////////////////////////////////////////////////////
// access to one global ImpTimedRefDev incarnation in namespace drawinglayer::primitive

namespace drawinglayer
{
    namespace primitive2d
    {
        // static methods here
        VirtualDevice& acquireGlobalVirtualDevice()
        {
            scoped_timed_RefDev& rStdRefDevice = the_scoped_timed_RefDev::get();

            if(!rStdRefDevice)
                rStdRefDevice.reset(new ImpTimedRefDev(rStdRefDevice));

            return rStdRefDevice->acquireVirtualDevice();
        }

        void releaseGlobalVirtualDevice()
        {
            scoped_timed_RefDev& rStdRefDevice = the_scoped_timed_RefDev::get();

            OSL_ENSURE(rStdRefDevice, "releaseGlobalVirtualDevice() without prior acquireGlobalVirtualDevice() call(!)");
            rStdRefDevice->releaseVirtualDevice();
        }

        TextLayouterDevice::TextLayouterDevice()
        :   mrDevice(acquireGlobalVirtualDevice())
        {
        }

        TextLayouterDevice::~TextLayouterDevice()
        {
            releaseGlobalVirtualDevice();
        }

        void TextLayouterDevice::setFont(const Font& rFont)
        {
            mrDevice.SetFont( rFont );
        }

        void TextLayouterDevice::setFontAttribute(
            const attribute::FontAttribute& rFontAttribute,
            double fFontScaleX,
            double fFontScaleY,
            const ::com::sun::star::lang::Locale& rLocale)
        {
            setFont(getVclFontFromFontAttribute(
                rFontAttribute,
                fFontScaleX,
                fFontScaleY,
                0.0,
                rLocale));
        }

        double TextLayouterDevice::getOverlineOffset() const
        {
            const ::FontMetric& rMetric = mrDevice.GetFontMetric();
            double fRet = (rMetric.GetIntLeading() / 2.0) - rMetric.GetAscent();
            return fRet;
        }

        double TextLayouterDevice::getUnderlineOffset() const
        {
            const ::FontMetric& rMetric = mrDevice.GetFontMetric();
            double fRet = rMetric.GetDescent() / 2.0;
            return fRet;
        }

        double TextLayouterDevice::getStrikeoutOffset() const
        {
            const ::FontMetric& rMetric = mrDevice.GetFontMetric();
            double fRet = (rMetric.GetAscent() - rMetric.GetIntLeading()) / 3.0;
            return fRet;
        }

        double TextLayouterDevice::getOverlineHeight() const
        {
            const ::FontMetric& rMetric = mrDevice.GetFontMetric();
            double fRet = rMetric.GetIntLeading() / 2.5;
            return fRet;
        }

        double TextLayouterDevice::getUnderlineHeight() const
        {
            const ::FontMetric& rMetric = mrDevice.GetFontMetric();
            double fRet = rMetric.GetDescent() / 4.0;
            return fRet;
        }

        double TextLayouterDevice::getTextHeight() const
        {
            return mrDevice.GetTextHeight();
        }

        double TextLayouterDevice::getTextWidth(
            const OUString& rText,
            sal_uInt32 nIndex,
            sal_uInt32 nLength) const
        {
            return mrDevice.GetTextWidth(rText, nIndex, nLength);
        }

        bool TextLayouterDevice::getTextOutlines(
            basegfx::B2DPolyPolygonVector& rB2DPolyPolyVector,
            const OUString& rText,
            sal_uInt32 nIndex,
            sal_uInt32 nLength,
            const ::std::vector< double >& rDXArray) const
        {
            const sal_uInt32 nDXArrayCount(rDXArray.size());
            sal_uInt32 nTextLength(nLength);
            const sal_uInt32 nStringLength(rText.getLength());

            if(nTextLength + nIndex > nStringLength)
            {
                nTextLength = nStringLength - nIndex;
            }

            if(nDXArrayCount)
            {
                OSL_ENSURE(nDXArrayCount == nTextLength, "DXArray size does not correspond to text portion size (!)");
                std::vector< sal_Int32 > aIntegerDXArray(nDXArrayCount);

                for(sal_uInt32 a(0); a < nDXArrayCount; a++)
                {
                    aIntegerDXArray[a] = basegfx::fround(rDXArray[a]);
                }

                return mrDevice.GetTextOutlines(
                    rB2DPolyPolyVector,
                    rText,
                    nIndex,
                    nIndex,
                    nLength,
                    true,
                    0,
                    &(aIntegerDXArray[0]));
            }
            else
            {
                return mrDevice.GetTextOutlines(
                    rB2DPolyPolyVector,
                    rText,
                    nIndex,
                    nIndex,
                    nLength,
                    true,
                    0,
                    0);
            }
        }

        basegfx::B2DRange TextLayouterDevice::getTextBoundRect(
            const OUString& rText,
            sal_uInt32 nIndex,
            sal_uInt32 nLength) const
        {
            sal_uInt32 nTextLength(nLength);
            const sal_uInt32 nStringLength(rText.getLength());

            if(nTextLength + nIndex > nStringLength)
            {
                nTextLength = nStringLength - nIndex;
            }

            if(nTextLength)
            {
                Rectangle aRect;

                mrDevice.GetTextBoundRect(
                    aRect,
                    rText,
                    nIndex,
                    nIndex,
                    nLength);

                // #i104432#, #i102556# take empty results into account
                if(!aRect.IsEmpty())
                {
                    return basegfx::B2DRange(
                        aRect.Left(), aRect.Top(),
                        aRect.Right(), aRect.Bottom());
                }
            }

            return basegfx::B2DRange();
        }

        double TextLayouterDevice::getFontAscent() const
        {
            const ::FontMetric& rMetric = mrDevice.GetFontMetric();
            return rMetric.GetAscent();
        }

        double TextLayouterDevice::getFontDescent() const
        {
            const ::FontMetric& rMetric = mrDevice.GetFontMetric();
            return rMetric.GetDescent();
        }

        void TextLayouterDevice::addTextRectActions(
            const Rectangle& rRectangle,
            const OUString& rText,
            sal_uInt16 nStyle,
            GDIMetaFile& rGDIMetaFile) const
        {
            mrDevice.AddTextRectActions(
                rRectangle, rText, nStyle, rGDIMetaFile);
        }

        ::std::vector< double > TextLayouterDevice::getTextArray(
            const OUString& rText,
            sal_uInt32 nIndex,
            sal_uInt32 nLength) const
        {
            ::std::vector< double > aRetval;
            sal_uInt32 nTextLength(nLength);
            const sal_uInt32 nStringLength(rText.getLength());

            if(nTextLength + nIndex > nStringLength)
            {
                nTextLength = nStringLength - nIndex;
            }

            if(nTextLength)
            {
                aRetval.reserve(nTextLength);
                ::std::vector<sal_Int32> aArray(nTextLength);
                mrDevice.GetTextArray(rText, &aArray[0], nIndex, nLength);
                aRetval.assign(aArray.begin(), aArray.end());
            }

            return aRetval;
        }

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// helper methods for vcl font handling

namespace drawinglayer
{
    namespace primitive2d
    {
        Font getVclFontFromFontAttribute(
            const attribute::FontAttribute& rFontAttribute,
            double fFontScaleX,
            double fFontScaleY,
            double fFontRotation,
            const ::com::sun::star::lang::Locale& rLocale)
        {
            // detect FontScaling
            const sal_uInt32 nHeight(basegfx::fround(fabs(fFontScaleY)));
            const sal_uInt32 nWidth(basegfx::fround(fabs(fFontScaleX)));
            const bool bFontIsScaled(nHeight != nWidth);

#ifdef WIN32
            // for WIN32 systems, start with creating an unscaled font. If FontScaling
            // is wanted, that width needs to be adapted using FontMetric again to get a
            // width of the unscaled font
            Font aRetval(
                rFontAttribute.getFamilyName(),
                rFontAttribute.getStyleName(),
                Size(0, nHeight));
#else
            // for non-WIN32 systems things are easier since these accept a Font creation
            // with initially nWidth != nHeight for FontScaling. Despite that, use zero for
            // FontWidth when no scaling is used to explicitly have that zero when e.g. the
            // Font would be recorded in a MetaFile (The MetaFile FontAction WILL record a
            // set FontWidth; import that in a WIN32 system, and trouble is there)
            Font aRetval(
                rFontAttribute.getFamilyName(),
                rFontAttribute.getStyleName(),
                Size(bFontIsScaled ? nWidth : 0, nHeight));
#endif
            // define various other FontAttribute
            aRetval.SetAlign(ALIGN_BASELINE);
            aRetval.SetCharSet(rFontAttribute.getSymbol() ? RTL_TEXTENCODING_SYMBOL : RTL_TEXTENCODING_UNICODE);
            aRetval.SetVertical(rFontAttribute.getVertical() ? sal_True : sal_False);
            aRetval.SetWeight(static_cast<FontWeight>(rFontAttribute.getWeight()));
            aRetval.SetItalic(rFontAttribute.getItalic() ? ITALIC_NORMAL : ITALIC_NONE);
            aRetval.SetOutline(rFontAttribute.getOutline());
            aRetval.SetPitch(rFontAttribute.getMonospaced() ? PITCH_FIXED : PITCH_VARIABLE);
            aRetval.SetLanguage(LanguageTag::convertToLanguageType( rLocale, false));

#ifdef WIN32
            // for WIN32 systems, correct the FontWidth if FontScaling is used
            if(bFontIsScaled && nHeight > 0)
            {
                const FontMetric aUnscaledFontMetric(Application::GetDefaultDevice()->GetFontMetric(aRetval));

                if(aUnscaledFontMetric.GetWidth() > 0)
                {
                    const double fScaleFactor((double)nWidth / (double)nHeight);
                    const sal_uInt32 nScaledWidth(basegfx::fround((double)aUnscaledFontMetric.GetWidth() * fScaleFactor));
                    aRetval.SetWidth(nScaledWidth);
                }
            }
#endif
            // handle FontRotation (if defined)
            if(!basegfx::fTools::equalZero(fFontRotation))
            {
                sal_Int16 aRotate10th((sal_Int16)(fFontRotation * (-1800.0/F_PI)));
                aRetval.SetOrientation(aRotate10th % 3600);
            }

            return aRetval;
        }

        attribute::FontAttribute getFontAttributeFromVclFont(
            basegfx::B2DVector& o_rSize,
            const Font& rFont,
            bool bRTL,
            bool bBiDiStrong)
        {
            const attribute::FontAttribute aRetval(
                rFont.GetName(),
                rFont.GetStyleName(),
                static_cast<sal_uInt16>(rFont.GetWeight()),
                RTL_TEXTENCODING_SYMBOL == rFont.GetCharSet(),
                rFont.IsVertical(),
                ITALIC_NONE != rFont.GetItalic(),
                PITCH_FIXED == rFont.GetPitch(),
                rFont.IsOutline(),
                bRTL,
                bBiDiStrong);
            // TODO: eKerning

            // set FontHeight and init to no FontScaling
            o_rSize.setY(rFont.GetSize().getHeight() > 0 ? rFont.GetSize().getHeight() : 0);
            o_rSize.setX(o_rSize.getY());

#ifdef WIN32
            // for WIN32 systems, the FontScaling at the Font is detected by
            // checking that FontWidth != 0. When FontScaling is used, WIN32
            // needs to do extra stuff to detect the correct width (since it's
            // zero and not equal the font height) and it's relationship to
            // the height
            if(rFont.GetSize().getWidth() > 0)
            {
                Font aUnscaledFont(rFont);
                aUnscaledFont.SetWidth(0);
                const FontMetric aUnscaledFontMetric(Application::GetDefaultDevice()->GetFontMetric(aUnscaledFont));

                if(aUnscaledFontMetric.GetWidth() > 0)
                {
                    const double fScaleFactor((double)rFont.GetSize().getWidth() / (double)aUnscaledFontMetric.GetWidth());
                    o_rSize.setX(fScaleFactor * o_rSize.getY());
                }
            }
#else
            // For non-WIN32 systems the detection is the same, but the value
            // is easier achieved since width == height is interpreted as no
            // scaling. Ergo, Width == 0 means width == height, and width != 0
            // means the scaling is in the direct relation of width to height
            if(rFont.GetSize().getWidth() > 0)
            {
                o_rSize.setX((double)rFont.GetSize().getWidth());
            }
#endif
            return aRetval;
        }
    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
