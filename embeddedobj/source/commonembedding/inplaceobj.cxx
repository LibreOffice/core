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
        throw embed::WrongStateException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object is not activated inplace!\n" )),
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
