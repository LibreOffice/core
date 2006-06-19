/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: inplaceobj.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 00:28:17 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_EMBED_EMBEDSTATES_HPP_
#include <com/sun/star/embed/EmbedStates.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

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
                                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

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

