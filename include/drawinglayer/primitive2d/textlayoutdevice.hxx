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

#ifndef INCLUDED_DRAWINGLAYER_TEXTLAYOUTDEVICE_HXX
#define INCLUDED_DRAWINGLAYER_TEXTLAYOUTDEVICE_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <tools/solar.h>
#include <tools/poly.hxx>
#include <basegfx/range/b2drange.hxx>
#include <vector>
#include <com/sun/star/lang/Locale.hpp>
#include <basegfx/polygon/b2dpolypolygon.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines
class VirtualDevice;
class Font;
namespace rtl {
   class OUString;
};
class OutputDevice;
class GDIMetaFile;

namespace drawinglayer { namespace attribute {
    class FontAttribute;
}}

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
                const rtl::OUString& rText,
                sal_uInt32 nIndex,
                sal_uInt32 nLength) const;

            bool getTextOutlines(
                basegfx::B2DPolyPolygonVector&,
                const rtl::OUString& rText,
                sal_uInt32 nIndex,
                sal_uInt32 nLength,
                const ::std::vector< double >& rDXArray) const;

            basegfx::B2DRange getTextBoundRect(
                const rtl::OUString& rText,
                sal_uInt32 nIndex,
                sal_uInt32 nLength) const;

            double getFontAscent() const;
            double getFontDescent() const;

            void addTextRectActions(
                const Rectangle& rRectangle,
                const rtl::OUString& rText,
                sal_uInt16 nStyle,
                GDIMetaFile& rGDIMetaFile) const;

            ::std::vector< double > getTextArray(
                const rtl::OUString& rText,
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
