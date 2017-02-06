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

#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/EmbedVerbs.hpp>
#include <com/sun/star/embed/EmbedUpdateModes.hpp>
#include <com/sun/star/embed/UnreachableStateException.hpp>
#include <com/sun/star/embed/WrongStateException.hpp>
#include <com/sun/star/embed/XEmbeddedClient.hpp>
#include <com/sun/star/embed/XInplaceClient.hpp>
#include <com/sun/star/embed/XWindowSupplier.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/util/XCloseBroadcaster.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/embed/EmbedMapUnits.hpp>

#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>

#include "specialobject.hxx"
#include "intercept.hxx"

using namespace ::com::sun::star;


OSpecialEmbeddedObject::OSpecialEmbeddedObject( const uno::Reference< uno::XComponentContext >& rxContext, const uno::Sequence< beans::NamedValue >& aObjectProps )
: OCommonEmbeddedObject( rxContext, aObjectProps )
{
    maSize.Width = maSize.Height = 10000;
    m_nObjectState = embed::EmbedStates::LOADED;
}


uno::Any SAL_CALL OSpecialEmbeddedObject::queryInterface( const uno::Type& rType )
{
    uno::Any aReturn;

    aReturn = ::cppu::queryInterface( rType,
                                        static_cast< embed::XEmbeddedObject* >( this ),
                                        static_cast< embed::XInplaceObject* >( this ),
                                        static_cast< embed::XVisualObject* >( this ),
                                        static_cast< embed::XClassifiedObject* >( this ),
                                        static_cast< embed::XComponentSupplier* >( this ),
                                        static_cast< util::XCloseable* >( this ),
                                        static_cast< document::XEventBroadcaster* >( this ) );
    if ( aReturn.hasValue() )
        return aReturn;
    else
        return ::cppu::OWeakObject::queryInterface( rType ) ;

}


embed::VisualRepresentation SAL_CALL OSpecialEmbeddedObject::getPreferredVisualRepresentation( sal_Int64 nAspect )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    // TODO: if object is in loaded state it should switch itself to the running state
    if ( m_nObjectState == -1 || m_nObjectState == embed::EmbedStates::LOADED )
        throw embed::WrongStateException( "The own object has no model!",
                                    static_cast< ::cppu::OWeakObject* >(this) );

    OSL_ENSURE( nAspect != embed::Aspects::MSOLE_ICON, "For iconified objects no graphical replacement is required!\n" );
    if ( nAspect == embed::Aspects::MSOLE_ICON )
        // no representation can be retrieved
        throw embed::WrongStateException( "Illegal call!",
                                    static_cast< ::cppu::OWeakObject* >(this) );

    // TODO: return for the aspect of the document
    embed::VisualRepresentation aVisualRepresentation;
    return aVisualRepresentation;
}

void SAL_CALL OSpecialEmbeddedObject::setVisualAreaSize( sal_Int64 nAspect, const awt::Size& aSize )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    OSL_ENSURE( nAspect != embed::Aspects::MSOLE_ICON, "For iconified objects no graphical replacement is required!\n" );
    if ( nAspect == embed::Aspects::MSOLE_ICON )
        // no representation can be retrieved
        throw embed::WrongStateException( "Illegal call!",
                                    static_cast< ::cppu::OWeakObject* >(this) );

    maSize = aSize;
}

awt::Size SAL_CALL OSpecialEmbeddedObject::getVisualAreaSize( sal_Int64 nAspect )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    OSL_ENSURE( nAspect != embed::Aspects::MSOLE_ICON, "For iconified objects no graphical replacement is required!\n" );
    if ( nAspect == embed::Aspects::MSOLE_ICON )
        // no representation can be retrieved
        throw embed::WrongStateException( "Illegal call!",
                                    static_cast< ::cppu::OWeakObject* >(this) );

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( "The own object has no model!",
                                    static_cast< ::cppu::OWeakObject* >(this) );

    return maSize;
}

sal_Int32 SAL_CALL OSpecialEmbeddedObject::getMapUnit( sal_Int64 nAspect )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    OSL_ENSURE( nAspect != embed::Aspects::MSOLE_ICON, "For iconified objects no graphical replacement is required!\n" );
    if ( nAspect == embed::Aspects::MSOLE_ICON )
        // no representation can be retrieved
        throw embed::WrongStateException( "Illegal call!",
                                    static_cast< ::cppu::OWeakObject* >(this) );

    return embed::EmbedMapUnits::ONE_100TH_MM;
}

void SAL_CALL OSpecialEmbeddedObject::changeState( sal_Int32 nNewState )
{
    if ( nNewState == embed::EmbedStates::UI_ACTIVE )
        nNewState = embed::EmbedStates::INPLACE_ACTIVE;
    OCommonEmbeddedObject::changeState( nNewState );
}

void SAL_CALL OSpecialEmbeddedObject::doVerb( sal_Int32 nVerbID )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( "The object has no persistence!",
                                        static_cast< ::cppu::OWeakObject* >(this) );

    if ( nVerbID == -7 )
    {

        uno::Reference < ui::dialogs::XExecutableDialog > xDlg( m_xDocHolder->GetComponent(), uno::UNO_QUERY );
        if ( xDlg.is() )
            xDlg->execute();
        else
            throw embed::UnreachableStateException();
    }
    else
        OCommonEmbeddedObject::doVerb( nVerbID );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
