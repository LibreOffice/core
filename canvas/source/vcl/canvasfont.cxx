/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: canvasfont.cxx,v $
 * $Revision: 1.9 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_canvas.hxx"

#include <canvas/debug.hxx>

#include <rtl/math.hxx>
#include <basegfx/numeric/ftools.hxx>

#include <vcl/metric.hxx>

#include "canvasfont.hxx"
#include "textlayout.hxx"

using namespace ::com::sun::star;


namespace vclcanvas
{
    CanvasFont::CanvasFont( const rendering::FontRequest&                   rFontRequest,
                            const uno::Sequence< beans::PropertyValue >&    ,
                            const geometry::Matrix2D&                       rFontMatrix,
                            const DeviceRef&                                rDevice ) :
        CanvasFont_Base( m_aMutex ),
        maFont( Font( rFontRequest.FontDescription.FamilyName,
                      rFontRequest.FontDescription.StyleName,
                      Size( 0, ::basegfx::fround(rFontRequest.CellSize) ) ) ),
        maFontRequest( rFontRequest ),
        mpRefDevice( rDevice )
    {
        maFont->SetAlign( ALIGN_BASELINE );
        maFont->SetCharSet( (rFontRequest.FontDescription.IsSymbolFont==com::sun::star::util::TriState_YES) ? RTL_TEXTENCODING_SYMBOL : RTL_TEXTENCODING_UNICODE );
        maFont->SetVertical( (rFontRequest.FontDescription.IsVertical==com::sun::star::util::TriState_YES) ? TRUE : FALSE );

        // TODO(F2): improve panose->vclenum conversion
        maFont->SetWeight( static_cast<FontWeight>(rFontRequest.FontDescription.FontDescription.Weight) );
        maFont->SetItalic( (rFontRequest.FontDescription.FontDescription.Letterform<=8) ? ITALIC_NONE : ITALIC_NORMAL );

        // adjust to stretched/shrinked font
        if( !::rtl::math::approxEqual( rFontMatrix.m00, rFontMatrix.m11) )
        {
            OutputDevice* pOutDev( mpRefDevice->getOutDev() );

            if( pOutDev )
            {
                const bool bOldMapState( pOutDev->IsMapModeEnabled() );
                pOutDev->EnableMapMode(FALSE);

                const Size aSize = pOutDev->GetFontMetric( *maFont ).GetSize();

                const double fDividend( rFontMatrix.m10 + rFontMatrix.m11 );
                double fStretch = (rFontMatrix.m00 + rFontMatrix.m01);

                if( !::basegfx::fTools::equalZero( fDividend) )
                    fStretch /= fDividend;

                const long nNewWidth = ::basegfx::fround( aSize.Width() * fStretch );

                maFont->SetWidth( nNewWidth );

                pOutDev->EnableMapMode(bOldMapState);
            }
        }
    }

    void SAL_CALL CanvasFont::disposing()
    {
        tools::LocalGuard aGuard;

        mpRefDevice.clear();
    }

    uno::Reference< rendering::XTextLayout > SAL_CALL  CanvasFont::createTextLayout( const rendering::StringContext& aText, sal_Int8 nDirection, sal_Int64 nRandomSeed ) throw (uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        if( !mpRefDevice.is() )
            return uno::Reference< rendering::XTextLayout >(); // we're disposed

        return new TextLayout( aText,
                               nDirection,
                               nRandomSeed,
                               Reference( this ),
                               mpRefDevice );
    }

    rendering::FontRequest SAL_CALL  CanvasFont::getFontRequest(  ) throw (uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        return maFontRequest;
    }

    rendering::FontMetrics SAL_CALL  CanvasFont::getFontMetrics(  ) throw (uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        // TODO(F1)
        return rendering::FontMetrics();
    }

    uno::Sequence< double > SAL_CALL  CanvasFont::getAvailableSizes(  ) throw (uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        // TODO(F1)
        return uno::Sequence< double >();
    }

    uno::Sequence< beans::PropertyValue > SAL_CALL  CanvasFont::getExtraFontProperties(  ) throw (uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        // TODO(F1)
        return uno::Sequence< beans::PropertyValue >();
    }

#define IMPLEMENTATION_NAME "VCLCanvas::CanvasFont"
#define SERVICE_NAME "com.sun.star.rendering.CanvasFont"

    ::rtl::OUString SAL_CALL CanvasFont::getImplementationName() throw( uno::RuntimeException )
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( IMPLEMENTATION_NAME ) );
    }

    sal_Bool SAL_CALL CanvasFont::supportsService( const ::rtl::OUString& ServiceName ) throw( uno::RuntimeException )
    {
        return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME ) );
    }

    uno::Sequence< ::rtl::OUString > SAL_CALL CanvasFont::getSupportedServiceNames()  throw( uno::RuntimeException )
    {
        uno::Sequence< ::rtl::OUString > aRet(1);
        aRet[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );

        return aRet;
    }

    ::Font CanvasFont::getVCLFont() const
    {
        return *maFont;
    }
}
