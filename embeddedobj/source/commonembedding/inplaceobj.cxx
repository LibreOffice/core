/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: inplaceobj.cxx,v $
 * $Revision: 1.7 $
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
#include "precompiled_embeddedobj.hxx"
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

#include "commonembobj.hxx"


using namespace ::com::sun::star;

awt::Rectangle GetRectangleInterception( const awt::Rectangle& aRect1, const awt::Rectangle& aRect2 );
sal_Bool RectanglesEqual( const awt::Rectangle& aRect1, const awt::Rectangle& aRect2 )
{
    return ( aRect1.X == aRect2.X
            && aRect1.Y == aRect2.Y
            && aRect1.Width == aRect2.Width
            && aRect1.Height == aRect2.Height );
}

void SAL_CALL OCommonEmbeddedObject::setObjectRectangles( const awt::Rectangle& aPosRect,
                                                           const awt::Rectangle& aClipRect )
        throw ( embed::WrongStateException,
                uno::Exception,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState != embed::EmbedStates::INPLACE_ACTIVE
      && m_nObjectState != embed::EmbedStates::UI_ACTIVE )
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The object is not activated inplace!\n" ),
                                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    awt::Rectangle aNewRectToShow = GetRectangleInterception( aPosRect, aClipRect );
    awt::Rectangle aOldRectToShow = GetRectangleInterception( m_aOwnRectangle, m_aClipRectangle );

    // the clip rectangle changes view only in case interception is also changed
    if ( !RectanglesEqual( m_aOwnRectangle, aPosRect )
      || ( !RectanglesEqual( m_aClipRectangle, aPosRect ) && !RectanglesEqual( aOldRectToShow, aNewRectToShow ) ) )
        m_pDocHolder->PlaceFrame( aNewRectToShow );

    m_aOwnRectangle = aPosRect;
    m_aClipRectangle = aClipRect;
}

void SAL_CALL OCommonEmbeddedObject::enableModeless( sal_Bool /*bEnable*/ )
        throw ( embed::WrongStateException,
                uno::Exception,
                uno::RuntimeException )
{
    // TODO: notify model that it can not use modal dialogs
}

void SAL_CALL OCommonEmbeddedObject::translateAccelerators(
                    const uno::Sequence< awt::KeyEvent >& /*aKeys*/ )
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    // TODO: UI activation related
}

