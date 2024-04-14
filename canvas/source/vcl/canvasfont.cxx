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
#include <canvas/canvastools.hxx>
#include <com/sun/star/rendering/PanoseProportion.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <rtl/math.hxx>
#include <vcl/metric.hxx>
#include <vcl/virdev.hxx>

#include "canvasfont.hxx"
#include "textlayout.hxx"

using namespace ::com::sun::star;


namespace vclcanvas
{
    CanvasFont::CanvasFont( const rendering::FontRequest&                   rFontRequest,
                            const uno::Sequence< beans::PropertyValue >&    rExtraFontProperties,
                            const geometry::Matrix2D&                       rFontMatrix,
                            rendering::XGraphicDevice&                      rDevice,
                            const OutDevProviderSharedPtr&                  rOutDevProvider ) :
        CanvasFont_Base( m_aMutex ),
        maFont( vcl::Font( rFontRequest.FontDescription.FamilyName,
                      rFontRequest.FontDescription.StyleName,
                      Size( 0, ::basegfx::fround<::tools::Long>(rFontRequest.CellSize) ) ) ),
        maFontRequest( rFontRequest ),
        mpRefDevice( &rDevice ),
        mpOutDevProvider( rOutDevProvider ),
        maFontMatrix( rFontMatrix )
    {
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
        tools::setupFontWidth(rFontMatrix, maFont.get(), rOutDevProvider->getOutDev());

        sal_uInt32 nEmphasisMark = 0;

        ::canvas::tools::extractExtraFontProperties(rExtraFontProperties, nEmphasisMark);

        if (nEmphasisMark)
            maFont->SetEmphasisMark(FontEmphasisMark(nEmphasisMark));
    }

    void SAL_CALL CanvasFont::disposing()
    {
        SolarMutexGuard aGuard;

        mpOutDevProvider.reset();
        mpRefDevice.clear();
    }

    uno::Reference< rendering::XTextLayout > SAL_CALL  CanvasFont::createTextLayout( const rendering::StringContext& aText, sal_Int8 nDirection, sal_Int64 )
    {
        SolarMutexGuard aGuard;

        if( !mpRefDevice.is() )
            return uno::Reference< rendering::XTextLayout >(); // we're disposed

        return new TextLayout( aText,
                               nDirection,
                               Reference( this ),
                               mpRefDevice,
                               mpOutDevProvider);
    }

    rendering::FontRequest SAL_CALL  CanvasFont::getFontRequest(  )
    {
        SolarMutexGuard aGuard;

        return maFontRequest;
    }

    rendering::FontMetrics SAL_CALL  CanvasFont::getFontMetrics(  )
    {
        SolarMutexGuard aGuard;

        OutputDevice& rOutDev = mpOutDevProvider->getOutDev();
        ScopedVclPtrInstance< VirtualDevice > pVDev( rOutDev );
        pVDev->SetFont(getVCLFont());
        const ::FontMetric& aMetric( pVDev->GetFontMetric() );

        return rendering::FontMetrics(
            aMetric.GetAscent(),
            aMetric.GetDescent(),
            aMetric.GetInternalLeading(),
            aMetric.GetExternalLeading(),
            0,
            aMetric.GetDescent() / 2.0,
            aMetric.GetAscent() / 2.0);
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
        return "VCLCanvas::CanvasFont";
    }

    sal_Bool SAL_CALL CanvasFont::supportsService( const OUString& ServiceName )
    {
        return cppu::supportsService( this, ServiceName );
    }

    uno::Sequence< OUString > SAL_CALL CanvasFont::getSupportedServiceNames()
    {
        return { "com.sun.star.rendering.CanvasFont" };
    }

    vcl::Font const & CanvasFont::getVCLFont() const
    {
        return *maFont;
    }

    const css::geometry::Matrix2D& CanvasFont::getFontMatrix() const
    {
        SolarMutexGuard aGuard;

        return maFontMatrix;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
