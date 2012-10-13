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


#include <ctype.h> // don't ask. msdev breaks otherwise...
#include "dx_winstuff.hxx"
#include "dx_spritecanvas.hxx"
#include "dx_canvasfont.hxx"
#include "dx_textlayout.hxx"

#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <com/sun/star/rendering/PanoseWeight.hpp>

using namespace ::com::sun::star;

namespace dxcanvas
{
    namespace
    {
        INT calcFontStyle( const rendering::FontRequest& rFontRequest )
        {
            INT nFontStyle( Gdiplus::FontStyleRegular );

            if( rFontRequest.FontDescription.FontDescription.Weight > rendering::PanoseWeight::BOOK )
                nFontStyle = Gdiplus::FontStyleBold;

            return nFontStyle;
        }
    }

    CanvasFont::CanvasFont( const rendering::FontRequest&                   rFontRequest,
                            const uno::Sequence< beans::PropertyValue >&    /*extraFontProperties*/,
                            const geometry::Matrix2D&                       fontMatrix ) :
        CanvasFont_Base( m_aMutex ),
        mpGdiPlusUser( GDIPlusUser::createInstance() ),
        // TODO(F1): extraFontProperties, fontMatrix
        mpFontFamily(),
        mpFont(),
        maFontRequest( rFontRequest ),
        maFontMatrix( fontMatrix )
    {
        const sal_Int32            nLen(rFontRequest.FontDescription.FamilyName.getLength());
        const sal_Unicode*         pStr(rFontRequest.FontDescription.FamilyName.getStr());
        std::vector< sal_Unicode > pStrBuf(nLen+1,0);
        std::copy(pStr,pStr+nLen,&pStrBuf[0]);

        mpFontFamily.reset( new Gdiplus::FontFamily(reinterpret_cast<LPCWSTR>(&pStrBuf[0]),NULL) );
        if( !mpFontFamily->IsAvailable() )
            mpFontFamily.reset( new Gdiplus::FontFamily(L"Arial",NULL) );

        mpFont.reset( new Gdiplus::Font( mpFontFamily.get(),
                                         static_cast<Gdiplus::REAL>(rFontRequest.CellSize),
                                         calcFontStyle( rFontRequest ),
                                         Gdiplus::UnitWorld ));
    }

    void SAL_CALL CanvasFont::disposing()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        mpFont.reset();
        mpFontFamily.reset();
        mpGdiPlusUser.reset();
    }

    uno::Reference< rendering::XTextLayout > SAL_CALL CanvasFont::createTextLayout( const rendering::StringContext& aText,
                                                                                    sal_Int8                        nDirection,
                                                                                    sal_Int64                       nRandomSeed ) throw (uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        return new TextLayout( aText, nDirection, nRandomSeed, ImplRef( this ) );
    }

    uno::Sequence< double > SAL_CALL CanvasFont::getAvailableSizes(  ) throw (uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // TODO
        return uno::Sequence< double >();
    }

    uno::Sequence< beans::PropertyValue > SAL_CALL CanvasFont::getExtraFontProperties(  ) throw (uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // TODO
        return uno::Sequence< beans::PropertyValue >();
    }

    rendering::FontRequest SAL_CALL CanvasFont::getFontRequest(  ) throw (uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        return maFontRequest;
    }

    rendering::FontMetrics SAL_CALL CanvasFont::getFontMetrics(  ) throw (uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // TODO
        return rendering::FontMetrics();
    }

#define SERVICE_NAME "com.sun.star.rendering.CanvasFont"
#define IMPLEMENTATION_NAME "DXCanvas::CanvasFont"

    ::rtl::OUString SAL_CALL CanvasFont::getImplementationName() throw( uno::RuntimeException )
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( IMPLEMENTATION_NAME ) );
    }

    sal_Bool SAL_CALL CanvasFont::supportsService( const ::rtl::OUString& ServiceName ) throw( uno::RuntimeException )
    {
        return ServiceName == SERVICE_NAME;
    }

    uno::Sequence< ::rtl::OUString > SAL_CALL CanvasFont::getSupportedServiceNames()  throw( uno::RuntimeException )
    {
        uno::Sequence< ::rtl::OUString > aRet(1);
        aRet[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );

        return aRet;
    }

    double CanvasFont::getCellAscent() const
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        return mpFontFamily->GetCellAscent(0); // TODO(F1): rFontRequest.styleName
    }

    double CanvasFont::getEmHeight() const
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        return mpFontFamily->GetEmHeight(0); // TODO(F1): rFontRequest.styleName
    }

    FontSharedPtr CanvasFont::getFont() const
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        return mpFont;
    }

    const ::com::sun::star::geometry::Matrix2D& CanvasFont::getFontMatrix() const
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        return maFontMatrix;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
