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

#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/WrongStateException.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/DisposedException.hpp>


#include <commonembobj.hxx>


using namespace ::com::sun::star;

void SAL_CALL OCommonEmbeddedObject::setVisualAreaSize( sal_Int64 nAspect, const awt::Size& aSize )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    SAL_WARN_IF( nAspect == embed::Aspects::MSOLE_ICON, "embeddedobj.common", "For iconified objects no graphical replacement is required!" );
    if ( nAspect == embed::Aspects::MSOLE_ICON )
        // no representation can be retrieved
        throw embed::WrongStateException( "Illegal call!",
                                    static_cast< ::cppu::OWeakObject* >(this) );

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( "The own object has no persistence!",
                                    static_cast< ::cppu::OWeakObject* >(this) );

    m_bHasClonedSize = false;

    bool bBackToLoaded = false;
    if ( m_nObjectState == embed::EmbedStates::LOADED )
    {
        changeState( embed::EmbedStates::RUNNING );

        // the links should be switched back to loaded state for now to avoid locking problems
        bBackToLoaded = m_bIsLink;
    }

    bool bSuccess = m_xDocHolder->SetExtent( nAspect, aSize );

    if ( bBackToLoaded )
        changeState( embed::EmbedStates::LOADED );

    if ( !bSuccess )
        throw uno::Exception(); // TODO:
}

awt::Size SAL_CALL OCommonEmbeddedObject::getVisualAreaSize( sal_Int64 nAspect )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( "The own object has no persistence!",
                                    static_cast< ::cppu::OWeakObject* >(this) );

    SAL_WARN_IF( nAspect == embed::Aspects::MSOLE_ICON, "embeddedobj.common", "For iconified objects no graphical replacement is required!" );

    if ( m_bHasClonedSize )
        return m_aClonedSize;

    bool bBackToLoaded = false;
    if ( m_nObjectState == embed::EmbedStates::LOADED )
    {
        changeState( embed::EmbedStates::RUNNING );

        // the links should be switched back to loaded state for now to avoid locking problems
        bBackToLoaded = m_bIsLink;
    }

    awt::Size aResult;
    bool bSuccess = m_xDocHolder->GetExtent( nAspect, &aResult );

    if ( bBackToLoaded )
        changeState( embed::EmbedStates::LOADED );

    if ( !bSuccess )
        throw uno::Exception(); // TODO:

    return aResult;
}

sal_Int32 SAL_CALL OCommonEmbeddedObject::getMapUnit( sal_Int64 nAspect )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    SAL_WARN_IF( nAspect == embed::Aspects::MSOLE_ICON, "embeddedobj.common", "For iconified objects no graphical replacement is required!" );
    if ( nAspect == embed::Aspects::MSOLE_ICON )
        // no representation can be retrieved
        throw embed::WrongStateException( "Illegal call!",
                                    static_cast< ::cppu::OWeakObject* >(this) );

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( "The own object has no persistence!",
                                    static_cast< ::cppu::OWeakObject* >(this) );

    if ( m_bHasClonedSize )
        return m_nClonedMapUnit;

    bool bBackToLoaded = false;
    if ( m_nObjectState == embed::EmbedStates::LOADED )
    {
        changeState( embed::EmbedStates::RUNNING );

        // the links should be switched back to loaded state for now to avoid locking problems
        bBackToLoaded = m_bIsLink;
    }

    sal_Int32 nResult = m_xDocHolder->GetMapUnit( nAspect );

    if ( bBackToLoaded )
        changeState( embed::EmbedStates::LOADED );

    if ( nResult < 0  )
        throw uno::Exception(); // TODO:

    return nResult;
}

embed::VisualRepresentation SAL_CALL OCommonEmbeddedObject::getPreferredVisualRepresentation( sal_Int64 nAspect )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( "The own object has no persistence!",
                                    static_cast< ::cppu::OWeakObject* >(this) );


    SAL_WARN_IF( nAspect == embed::Aspects::MSOLE_ICON, "embeddedobj.common", "For iconified objects no graphical replacement is required!" );
    if ( nAspect == embed::Aspects::MSOLE_ICON )
        // no representation can be retrieved
        throw embed::WrongStateException( "Illegal call!",
                                    static_cast< ::cppu::OWeakObject* >(this) );

    bool bBackToLoaded = false;
    if ( m_nObjectState == embed::EmbedStates::LOADED )
    {
        // restore original VisualAreaSize, because writer objects set
        // themselves to a default size OLESIZE
        awt::Size aOrigSize = getVisualAreaSize(nAspect);
        changeState(embed::EmbedStates::RUNNING);
        if (aOrigSize != getVisualAreaSize(nAspect))
            setVisualAreaSize(nAspect, aOrigSize);

        // the links should be switched back to loaded state for now to avoid locking problems
        bBackToLoaded = m_bIsLink;
    }

    SAL_WARN_IF( !m_xDocHolder->GetComponent().is(), "embeddedobj.common", "Running or Active object has no component!" );

    // TODO: return for the aspect of the document
    embed::VisualRepresentation aVisualRepresentation;

    uno::Reference< embed::XVisualObject > xVisualObject( m_xDocHolder->GetComponent(), uno::UNO_QUERY );
    if( xVisualObject.is())
    {
        aVisualRepresentation = xVisualObject->getPreferredVisualRepresentation( nAspect );
    }
    else
    {
        uno::Reference< datatransfer::XTransferable > xTransferable( m_xDocHolder->GetComponent(), uno::UNO_QUERY );
        if (!xTransferable.is() )
            throw uno::RuntimeException();

        datatransfer::DataFlavor aDataFlavor(
                "application/x-openoffice-gdimetafile;windows_formatname=\"GDIMetaFile\"",
                "GDIMetaFile",
                cppu::UnoType<uno::Sequence< sal_Int8 >>::get() );

        if( xTransferable->isDataFlavorSupported( aDataFlavor ))
        {
            aVisualRepresentation.Data = xTransferable->getTransferData( aDataFlavor );
            aVisualRepresentation.Flavor = aDataFlavor;
        }
        else
            throw uno::RuntimeException();
    }

    if ( bBackToLoaded )
        changeState( embed::EmbedStates::LOADED );

    return aVisualRepresentation;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
