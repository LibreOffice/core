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

#include "null_canvasfont.hxx"
#include "null_textlayout.hxx"
#include "null_spritecanvas.hxx"

#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <com/sun/star/rendering/PanoseWeight.hpp>

using namespace ::com::sun::star;

namespace nullcanvas
{
    CanvasFont::CanvasFont( const rendering::FontRequest&                   rFontRequest,
                            const uno::Sequence< beans::PropertyValue >&    /*extraFontProperties*/,
                            const geometry::Matrix2D&                       fontMatrix ) :
        CanvasFont_Base( m_aMutex ),
        maFontRequest( rFontRequest ),
        maFontMatrix( fontMatrix )
    {
    }

    void SAL_CALL CanvasFont::disposing()
    {
        ::osl::MutexGuard aGuard( m_aMutex );
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
#define IMPLEMENTATION_NAME "NullCanvas::CanvasFont"

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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
