/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef INCLUDED_DRAWINGLAYER_TEXTLAYOUTDEVICE_HXX
#define INCLUDED_DRAWINGLAYER_TEXTLAYOUTDEVICE_HXX

#include <drawinglayer/drawinglayerdllapi.h>

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
class GDIMetaFile;

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
        class DRAWINGLAYER_DLLPUBLIC TextLayouterDevice
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
                sal_uInt32 nIndex,
                sal_uInt32 nLength) const;

            bool getTextOutlines(
                basegfx::B2DPolyPolygonVector&,
                const String& rText,
                sal_uInt32 nIndex,
                sal_uInt32 nLength,
                const ::std::vector< double >& rDXArray) const;

            basegfx::B2DRange getTextBoundRect(
                const String& rText,
                sal_uInt32 nIndex,
                sal_uInt32 nLength) const;

            double getFontAscent() const;
            double getFontDescent() const;

            void addTextRectActions(
                const Rectangle& rRectangle,
                const String& rText,
                sal_uInt16 nStyle,
                GDIMetaFile& rGDIMetaFile) const;

            ::std::vector< double > getTextArray(
                const String& rText,
                sal_uInt32 nIndex,
                sal_uInt32 nLength) const;
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
        Font DRAWINGLAYER_DLLPUBLIC getVclFontFromFontAttribute(
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
        attribute::FontAttribute DRAWINGLAYER_DLLPUBLIC getFontAttributeFromVclFont(
            basegfx::B2DVector& o_rSize,
            const Font& rFont,
            bool bRTL,
            bool bBiDiStrong);

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //_DRAWINGLAYER_TEXTLAYOUTDEVICE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
