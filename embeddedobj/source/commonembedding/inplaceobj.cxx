/*************************************************************************
 *
 *  $RCSfile: inplaceobj.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-10-04 19:49:13 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

void SAL_CALL OCommonEmbeddedObject::enableModeless( sal_Bool bEnable )
        throw ( embed::WrongStateException,
                uno::Exception,
                uno::RuntimeException )
{
    // TODO: notify model that it can not use modal dialogs
}

void SAL_CALL OCommonEmbeddedObject::translateAccelerators(
                    const uno::Sequence< awt::KeyEvent >& aKeys )
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    // TODO: UI activation related
}

