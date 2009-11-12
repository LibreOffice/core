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

namespace drawinglayer { namespace attribute {
    class FontAttribute;
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
        /** TextLayouterDevice class

            This helper class exists to isolate all accesses to VCL
            text formatting/handling functionality for primitive implementations.
            When in the future FontHandling may move to an own library independent
            from VCL, primitives will be prepared.
         */
        class TextLayouterDevice
        {
            /// internally used VirtualDevice
            VirtualDevice&                  mrDevice;

        public:
            /// constructor/destructor
            TextLayouterDevice();
            ~TextLayouterDevice();

            /// tooling methods
            void setFont(const Font& rFont);
            void setFontAttribute(
                const attribute::FontAttribute& rFontAttribute,
                double fFontScaleX,
                double fFontScaleY,
                const ::com::sun::star::lang::Locale & rLocale);

            double getTextHeight() const;
            double getOverlineHeight() const;
            double getOverlineOffset() const;
            double getUnderlineHeight() const;
            double getUnderlineOffset() const;
            double getStrikeoutOffset() const;

            double getTextWidth(
                const String& rText,
                xub_StrLen nIndex,
                xub_StrLen nLength) const;

            bool getTextOutlines(
                basegfx::B2DPolyPolygonVector&,
                const String& rText,
                xub_StrLen nIndex,
                xub_StrLen nLength,
                const ::std::vector< double >& rDXArray);

            basegfx::B2DRange getTextBoundRect(
                const String& rText,
                xub_StrLen nIndex,
                xub_StrLen nLength) const;

            double getFontAscent() const;
            double getFontDescent() const;
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// helper methods for vcl font handling

namespace drawinglayer
{
    namespace primitive2d
    {
        /** Create a VCL-Font based on the definitions in FontAttribute
            and the given FontScaling. The FontScaling defines the FontHeight
            (fFontScaleY) and the FontWidth (fFontScaleX). The combination of
            both defines FontStretching, where no stretching happens at
            fFontScaleY == fFontScaleX
         */
        Font getVclFontFromFontAttribute(
            const attribute::FontAttribute& rFontAttribute,
            double fFontScaleX,
            double fFontScaleY,
            double fFontRotation,
            const ::com::sun::star::lang::Locale & rLocale);

        /** Generate FontAttribute DataSet derived from the given VCL-Font.
            The FontScaling with fFontScaleY, fFontScaleX relationship (see
            above) will be set in return parameter o_rSize to allow further
            processing
         */
        attribute::FontAttribute getFontAttributeFromVclFont(
            basegfx::B2DVector& o_rSize,
            const Font& rFont,
            bool bRTL,
            bool bBiDiStrong);

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //_DRAWINGLAYER_TEXTLAYOUTDEVICE_HXX
