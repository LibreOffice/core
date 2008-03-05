/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textlayoutdevice.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: aw $ $Date: 2008-03-05 09:15:43 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#ifndef INCLUDED_DRAWINGLAYER_TEXTLAYOUTDEVICE_HXX
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#endif

#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif

#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif

#ifndef _SV_FONT_HXX
#include <vcl/font.hxx>
#endif

#ifndef _SV_METRIC_HXX
#include <vcl/metric.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#endif

//#ifndef _SV_OUTDEV_HXX
//#include <vcl/outdev.hxx>
//#endif

//////////////////////////////////////////////////////////////////////////////
// VDev RevDevice provider

namespace
{
    class ImpTimedRefDev : public Timer
    {
        ImpTimedRefDev**                    mppStaticPointerOnMe;
        VirtualDevice*                      mpVirDev;
        sal_uInt32                          mnUseCount;

    public:
        ImpTimedRefDev(ImpTimedRefDev** ppStaticPointerOnMe);
        ~ImpTimedRefDev();
        virtual void Timeout();

        VirtualDevice& acquireVirtualDevice();
        void releaseVirtualDevice();
    };

    ImpTimedRefDev::ImpTimedRefDev(ImpTimedRefDev** ppStaticPointerOnMe)
    :   mppStaticPointerOnMe(ppStaticPointerOnMe),
        mpVirDev(0L),
        mnUseCount(0L)
    {
        SetTimeout(3L * 60L * 1000L); // three minutes
        Start();
    }

    ImpTimedRefDev::~ImpTimedRefDev()
    {
        OSL_ENSURE(0L == mnUseCount, "destruction of a still used ImpTimedRefDev (!)");

        if(mppStaticPointerOnMe && *mppStaticPointerOnMe)
        {
            *mppStaticPointerOnMe = 0L;
        }

        if(mpVirDev)
        {
            delete mpVirDev;
        }
    }

    void ImpTimedRefDev::Timeout()
    {
        // for obvious reasons, do not call anything after this
        delete (this);
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
        // static pointer here
        static ImpTimedRefDev* pImpGlobalRefDev = 0L;

        // static methods here
        VirtualDevice& acquireGlobalVirtualDevice()
        {
            if(!pImpGlobalRefDev)
            {
                pImpGlobalRefDev = new ImpTimedRefDev(&pImpGlobalRefDev);
            }

            return pImpGlobalRefDev->acquireVirtualDevice();
        }

        void releaseGlobalVirtualDevice()
        {
            OSL_ENSURE(pImpGlobalRefDev, "releaseGlobalVirtualDevice() without prior acquireGlobalVirtualDevice() call(!)");
            pImpGlobalRefDev->releaseVirtualDevice();
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

        void TextLayouterDevice::setFontAttributes(const FontAttributes& rFontAttributes, const basegfx::B2DHomMatrix& rTransform)
        {
            setFont(getVclFontFromFontAttributes(rFontAttributes, rTransform, mrDevice));
        }

        void TextLayouterDevice::setFontAttributes(const FontAttributes& rFontAttributes, double fFontScaleX, double fFontScaleY)
        {
            setFont(getVclFontFromFontAttributes(rFontAttributes, fFontScaleX, fFontScaleY, 0.0, mrDevice));
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
            const String& rText,
            xub_StrLen nIndex,
            xub_StrLen nLength) const
        {
            return mrDevice.GetTextWidth(rText, nIndex, nLength);
        }

        bool TextLayouterDevice::getTextOutlines(
            basegfx::B2DPolyPolygonVector& rB2DPolyPolyVector,
            const String& rText,
            xub_StrLen nIndex,
            xub_StrLen nLength)
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

        basegfx::B2DRange TextLayouterDevice::getTextBoundRect(
            const String& rText,
            xub_StrLen nIndex,
            xub_StrLen nLength) const
        {
            if(nLength)
            {
                Rectangle aRect;

                mrDevice.GetTextBoundRect(
                    aRect,
                    rText,
                    nIndex,
                    nIndex,
                    nLength);

                return basegfx::B2DRange(aRect.Left(), aRect.Top(), aRect.Right(), aRect.Bottom());
            }
            else
            {
                return basegfx::B2DRange();
            }
        }
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// helper methods for vcl font handling

namespace drawinglayer
{
    namespace primitive2d
    {
        Font getVclFontFromFontAttributes(
            const FontAttributes& rFontAttributes,
            const basegfx::B2DHomMatrix& rTransform,
            const OutputDevice& rOutDev)
        {
            // decompose matrix to have position and size of text
            basegfx::B2DVector aScale, aTranslate;
            double fRotate, fShearX;

            rTransform.decompose(aScale, aTranslate, fRotate, fShearX);

            return getVclFontFromFontAttributes(rFontAttributes, aScale.getX(), aScale.getY(), fRotate, rOutDev);
        }

        Font getVclFontFromFontAttributes(
            const FontAttributes& rFontAttributes,
            double fFontScaleX,
            double fFontScaleY,
            double fFontRotation,
            const OutputDevice& rOutDev)
        {
#ifndef WIN32
            // not used under unix, but reference for warning-free
            (void)rOutDev;
#endif
            sal_uInt32 nWidth(basegfx::fround(fabs(fFontScaleX)));
            sal_uInt32 nHeight(basegfx::fround(fabs(fFontScaleY)));
            Font aRetval(
                rFontAttributes.getFamilyName(),
                rFontAttributes.getStyleName(),
#ifdef WIN32
                Size(0, nHeight));
#else
                Size(nWidth, nHeight));
#endif

            aRetval.SetAlign(ALIGN_BASELINE);
            aRetval.SetCharSet(rFontAttributes.getSymbol() ? RTL_TEXTENCODING_SYMBOL : RTL_TEXTENCODING_UNICODE);
            aRetval.SetVertical(rFontAttributes.getVertical() ? TRUE : FALSE);
            aRetval.SetWeight(static_cast<FontWeight>(rFontAttributes.getWeight()));
            aRetval.SetItalic(rFontAttributes.getItalic() ? ITALIC_NORMAL : ITALIC_NONE);
            aRetval.SetOutline(rFontAttributes.getOutline());

#ifdef WIN32
            if(nWidth != nHeight)
            {
                const FontMetric aFontMetric(rOutDev.GetFontMetric(aRetval));
                const double fCurrentWidth(aFontMetric.GetWidth());

                aRetval.SetWidth(basegfx::fround(fCurrentWidth * (nWidth/nHeight)));
            }
#endif

            if(!basegfx::fTools::equalZero(fFontRotation))
            {
                sal_Int16 aRotate10th((sal_Int16)(fFontRotation * (-1800.0/F_PI)));
                aRetval.SetOrientation(aRotate10th % 3600);
            }

            return aRetval;
        }

        FontAttributes getFontAttributesFromVclFont(basegfx::B2DVector& rSize, const Font& rFont, bool bRTL, bool bBiDiStrong)
        {
            FontAttributes aRetval(
                rFont.GetName(),
                rFont.GetStyleName(),
                static_cast<sal_uInt16>(rFont.GetWeight()),
                RTL_TEXTENCODING_SYMBOL == rFont.GetCharSet(),
                rFont.IsVertical(),
                ITALIC_NONE != rFont.GetItalic(),
                rFont.IsOutline(),
                bRTL,
                bBiDiStrong);
            // TODO: eKerning

            const sal_Int32 nWidth(rFont.GetSize().getWidth());
            const sal_Int32 nHeight(rFont.GetSize().getHeight());

            rSize.setX(nWidth ? nWidth : nHeight);
            rSize.setY(nHeight);

            return aRetval;
        }
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
