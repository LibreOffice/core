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

#include <basegfx/numeric/ftools.hxx>
#include <com/sun/star/rendering/PanoseProportion.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <rtl/math.hxx>
#include <utility>
#include <vcl/metric.hxx>

#include <canvas/canvastools.hxx>

#include "cairo_canvasfont.hxx"
#include "cairo_textlayout.hxx"

using namespace ::com::sun::star;

namespace cairocanvas
{

    CanvasFont::CanvasFont( const rendering::FontRequest&                   rFontRequest,
                            const uno::Sequence< beans::PropertyValue >&    rExtraFontProperties,
                            const geometry::Matrix2D&                       rFontMatrix,
                            SurfaceProviderRef                              rDevice ) :
        maFont( vcl::Font( rFontRequest.FontDescription.FamilyName,
                      rFontRequest.FontDescription.StyleName,
                      Size( 0, ::basegfx::fround(rFontRequest.CellSize) ) ) ),
        maFontRequest( rFontRequest ),
        mpRefDevice(std::move( rDevice )),
        mnEmphasisMark(0)
    {
        ::canvas::tools::extractExtraFontProperties(rExtraFontProperties, mnEmphasisMark);

        maFont->SetAlignment( ALIGN_BASELINE );
        maFont->SetCharSet( (rFontRequest.FontDescription.IsSymbolFont==css::util::TriState_YES) ? RTL_TEXTENCODING_SYMBOL : RTL_TEXTENCODING_UNICODE );
        maFont->SetVertical( rFontRequest.FontDescription.IsVertical==css::util::TriState_YES );

        // TODO(F2): improve panose->vclenum conversion
        maFont->SetWeight( static_cast<FontWeight>(rFontRequest.FontDescription.FontDescription.Weight) );
        maFont->SetItalic( (rFontRequest.FontDescription.FontDescription.Letterform<=8) ? ITALIC_NONE : ITALIC_NORMAL );
        maFont->SetPitch(
                rFontRequest.FontDescription.FontDescription.Proportion == rendering::PanoseProportion::MONO_SPACED
                    ? PITCH_FIXED : PITCH_VARIABLE);

        maFont->SetLanguage( LanguageTag::convertToLanguageType( rFontRequest.Locale, false));

        // adjust to stretched/shrunk font
        if( ::rtl::math::approxEqual( rFontMatrix.m00, rFontMatrix.m11) )
            return;

        VclPtr<OutputDevice> pOutDev( mpRefDevice->getOutputDevice() );

        if( !pOutDev )
            return;

        const bool bOldMapState( pOutDev->IsMapModeEnabled() );
        pOutDev->EnableMapMode(false);

        const Size aSize = pOutDev->GetFontMetric( *maFont ).GetFontSize();

        const double fDividend( rFontMatrix.m10 + rFontMatrix.m11 );
        double fStretch = rFontMatrix.m00 + rFontMatrix.m01;

        if( !::basegfx::fTools::equalZero( fDividend) )
            fStretch /= fDividend;

        const tools::Long nNewWidth = ::basegfx::fround( aSize.Width() * fStretch );

        maFont->SetAverageFontWidth( nNewWidth );

        pOutDev->EnableMapMode(bOldMapState);
    }

    void CanvasFont::disposing(std::unique_lock<std::mutex>& rGuard)
    {
        rGuard.unlock();
        {
            SolarMutexGuard aGuard;
            mpRefDevice.clear();
        }
        rGuard.lock();
    }

    uno::Reference< rendering::XTextLayout > SAL_CALL  CanvasFont::createTextLayout( const rendering::StringContext& aText, sal_Int8 nDirection, sal_Int64 nRandomSeed )
    {
        SolarMutexGuard aGuard;

        if( !mpRefDevice.is() )
            return uno::Reference< rendering::XTextLayout >(); // we're disposed

        return new TextLayout( aText,
                               nDirection,
                               nRandomSeed,
                               Reference( this ),
                               mpRefDevice );
    }

    rendering::FontRequest SAL_CALL  CanvasFont::getFontRequest(  )
    {
        return maFontRequest;
    }

    rendering::FontMetrics SAL_CALL  CanvasFont::getFontMetrics(  )
    {
        // TODO(F1)
        return rendering::FontMetrics();
    }

    uno::Sequence< double > SAL_CALL  CanvasFont::getAvailableSizes(  )
    {
        // TODO(F1)
        return uno::Sequence< double >();
    }

    uno::Sequence< beans::PropertyValue > SAL_CALL  CanvasFont::getExtraFontProperties(  )
    {
        // TODO(F1)
        return uno::Sequence< beans::PropertyValue >();
    }

    OUString SAL_CALL CanvasFont::getImplementationName()
    {
        return u"CairoCanvas::CanvasFont"_ustr;
    }

    sal_Bool SAL_CALL CanvasFont::supportsService( const OUString& ServiceName )
    {
        return cppu::supportsService( this, ServiceName );
    }

    uno::Sequence< OUString > SAL_CALL CanvasFont::getSupportedServiceNames()
    {
        return { u"com.sun.star.rendering.CanvasFont"_ustr };
    }

    vcl::Font const & CanvasFont::getVCLFont() const
    {
        return *maFont;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
