/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ogl_canvasfont.hxx"
#include "ogl_textlayout.hxx"

#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <com/sun/star/rendering/PanoseWeight.hpp>


using namespace ::com::sun::star;

namespace oglcanvas
{
    CanvasFont::CanvasFont( const rendering::FontRequest&                   rFontRequest,
                            const uno::Sequence< beans::PropertyValue >&    /*extraFontProperties*/,
                            const geometry::Matrix2D&                       fontMatrix ) :
        CanvasFontBaseT( m_aMutex ),
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
                                                                                    sal_Int64                       nRandomSeed ) throw (uno::RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        return new TextLayout( aText, nDirection, nRandomSeed, ImplRef( this ) );
    }

    uno::Sequence< double > SAL_CALL CanvasFont::getAvailableSizes(  ) throw (uno::RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // TODO
        return uno::Sequence< double >();
    }

    uno::Sequence< beans::PropertyValue > SAL_CALL CanvasFont::getExtraFontProperties(  ) throw (uno::RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // TODO
        return uno::Sequence< beans::PropertyValue >();
    }

    rendering::FontRequest SAL_CALL CanvasFont::getFontRequest(  ) throw (uno::RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        return maFontRequest;
    }

    rendering::FontMetrics SAL_CALL CanvasFont::getFontMetrics(  ) throw (uno::RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // TODO
        return rendering::FontMetrics();
    }

    const ::com::sun::star::geometry::Matrix2D& CanvasFont::getFontMatrix() const
    {
        return maFontMatrix;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
