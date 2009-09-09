/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textlayoutdevice.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: aw $ $Date: 2008-05-27 14:11:17 $
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

#ifndef INCLUDED_DRAWINGLAYER_TEXTLAYOUTDEVICE_HXX
#define INCLUDED_DRAWINGLAYER_TEXTLAYOUTDEVICE_HXX

#include <sal/types.h>
#include <tools/solar.h>
#include <tools/poly.hxx>
#include <basegfx/range/b2drange.hxx>
#include <vector>
#include <com/sun/star/lang/Locale.hpp>

//////////////////////////////////////////////////////////////////////////////
// predefines
class VirtualDevice;
class Font;
class String;
class OutputDevice;

namespace drawinglayer { namespace primitive2d {
    class FontAttributes;
}}

namespace basegfx {
    class B2DPolyPolygon;
    typedef ::std::vector< B2DPolyPolygon > B2DPolyPolygonVector;
}

//////////////////////////////////////////////////////////////////////////////
// access to one global impTimedRefDev incarnation in namespace drawinglayer::primitive

namespace drawinglayer
{
    namespace primitive2d
    {
        class TextLayouterDevice
        {
            // internally used VirtualDevice
            VirtualDevice&                  mrDevice;

        public:
            TextLayouterDevice();
            ~TextLayouterDevice();

            void setFont(const Font& rFont);
            void setFontAttributes(const FontAttributes& rFontAttributes, const basegfx::B2DHomMatrix& rTransform, const ::com::sun::star::lang::Locale & rLocale);
            void setFontAttributes(const FontAttributes& rFontAttributes, double fFontScaleX, double fFontScaleY, const ::com::sun::star::lang::Locale & rLocale);

            double getTextHeight() const;
            double getOverlineHeight() const;
            double getOverlineOffset() const;
            double getUnderlineHeight() const;
            double getUnderlineOffset() const;
            double getStrikeoutOffset() const;
#ifdef WIN32
            double getCurrentFontRelation() const;
#endif

            double getTextWidth(
                const String& rText,
                xub_StrLen nIndex,
                xub_StrLen nLength) const;

            bool getTextOutlines(
                basegfx::B2DPolyPolygonVector&,
                const String& rText,
                xub_StrLen nIndex,
                xub_StrLen nLength,
                // #i89784# added suppirt for DXArray for justified text
                const ::std::vector< double >& rDXArray,
                double fFontScaleWidth);

            basegfx::B2DRange getTextBoundRect(
                const String& rText,
                xub_StrLen nIndex,
                xub_StrLen nLength) const;
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        // helper methods for vcl font handling
        Font getVclFontFromFontAttributes(
            const FontAttributes& rFontAttributes,
            double fFontScaleX,
            double fFontScaleY,
            double fFontRotation,
            const ::com::sun::star::lang::Locale & rLocale,
            const OutputDevice& rOutDev);

        Font getVclFontFromFontAttributes(
            const FontAttributes& rFontAttributes,
            const basegfx::B2DHomMatrix& rTransform,
            const ::com::sun::star::lang::Locale & rLocale,
            const OutputDevice& rOutDev);

        FontAttributes getFontAttributesFromVclFont(basegfx::B2DVector& rSize, const Font& rFont, bool bRTL, bool bBiDiStrong);

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //_DRAWINGLAYER_TEXTLAYOUTDEVICE_HXX
