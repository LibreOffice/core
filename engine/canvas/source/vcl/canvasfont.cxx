/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <basegfx/numeric/ftools.hxx>
#include <canvastools.hxx>
#include <com/sun/star/rendering/PanoseProportion.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <rtl/math.hxx>
#include <vcl/metric.hxx>
#include <vcl/virdev.hxx>

#include <canvasfont.hxx>
#include <canvas.hxx>
#include <textlayout.hxx>

using namespace ::com::sun::star;


namespace vclcanvas
{
    CanvasFont::CanvasFont( const rendering::FontRequest&                   rFontRequest,
                            FontEmphasisMark                                eEmphasisMark,
                            const geometry::Matrix2D&                       rFontMatrix,
                            OutputDevice&                                   rOutDev ) :
        maFont( rFontRequest.FontDescription.FamilyName,
                rFontRequest.FontDescription.StyleName,
                Size( 0, ::basegfx::fround<::tools::Long>(rFontRequest.CellSize) ) ),
        mxOutDev( &rOutDev ),
        maFontMatrix( rFontMatrix )
  {
        maFont.SetAlignment( ALIGN_BASELINE );
        maFont.SetCharSet( (rFontRequest.FontDescription.IsSymbolFont==css::util::TriState_YES) ? RTL_TEXTENCODING_SYMBOL : RTL_TEXTENCODING_UNICODE );
        maFont.SetVertical( rFontRequest.FontDescription.IsVertical==css::util::TriState_YES );

        // TODO(F2): improve panose->vclenum conversion
        maFont.SetWeight( static_cast<FontWeight>(rFontRequest.FontDescription.FontDescription.Weight) );
        maFont.SetItalic( (rFontRequest.FontDescription.FontDescription.Letterform<=8) ? ITALIC_NONE : ITALIC_NORMAL );
        maFont.SetPitch(
                rFontRequest.FontDescription.FontDescription.Proportion == rendering::PanoseProportion::MONO_SPACED
                    ? PITCH_FIXED : PITCH_VARIABLE);

        maFont.SetLanguage( LanguageTag::convertToLanguageType( rFontRequest.Locale, false));

        // adjust to stretched/shrunk font
        vclcanvastools::setupFontWidth(rFontMatrix, maFont, rOutDev);

        maFont.SetEmphasisMark(eEmphasisMark);
    }

    rtl::Reference< vclcanvas::TextLayout >  CanvasFont::createTextLayout( const rendering::StringContext& aText, sal_Int8 nDirection, sal_Int64 )
    {
        return new TextLayout( aText,
                               nDirection,
                               this,
                               mxOutDev);
    }

    vcl::Font const & CanvasFont::getVCLFont() const
    {
        return maFont;
    }

    const css::geometry::Matrix2D& CanvasFont::getFontMatrix() const
    {
        return maFontMatrix;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
