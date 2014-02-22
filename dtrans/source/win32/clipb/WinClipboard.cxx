/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <osl/diagnose.h>
#include "WinClipboard.hxx"
#include <com/sun/star/datatransfer/clipboard/ClipboardEvent.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <cppuhelper/supportsservice.hxx>
#include "WinClipbImpl.hxx"





using namespace osl;
using namespace std;
using namespace cppu;

using namespace com::sun::star::uno;
using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::clipboard;
using namespace com::sun::star::lang;


#define WINCLIPBOARD_IMPL_NAME  "com.sun.star.datatransfer.clipboard.ClipboardW32"





namespace
{
    Sequence< OUString > SAL_CALL WinClipboard_getSupportedServiceNames()
    {
        Sequence< OUString > aRet(1);
        aRet[0] = "com.sun.star.datatransfer.clipboard.SystemClipboard";
        return aRet;
    }
}




/*XEventListener,*/
CWinClipboard::CWinClipboard( const Reference< XComponentContext >& rxContext, const OUString& aClipboardName ) :
    WeakComponentImplHelper3< XSystemClipboard, XFlushableClipboard, XServiceInfo >( m_aCbListenerMutex ),
    m_xContext( rxContext )
{
    m_pImpl.reset( new CWinClipbImpl( aClipboardName, this ) );
}














Reference< XTransferable > SAL_CALL CWinClipboard::getContents( ) throw( RuntimeException )
{
    MutexGuard aGuard( m_aMutex );

    if ( rBHelper.bDisposed )
        throw DisposedException("object is already disposed",
                                 static_cast< XClipboardEx* >( this ) );

    if ( NULL != m_pImpl.get( ) )
        return m_pImpl->getContents( );

    return Reference< XTransferable >( );
}





void SAL_CALL CWinClipboard::setContents( const Reference< XTransferable >& xTransferable,
                                          const Reference< XClipboardOwner >& xClipboardOwner )
                                          throw( RuntimeException )
{
    MutexGuard aGuard( m_aMutex );

    if ( rBHelper.bDisposed )
        throw DisposedException("object is already disposed",
                                 static_cast< XClipboardEx* >( this ) );

    if ( NULL != m_pImpl.get( ) )
        m_pImpl->setContents( xTransferable, xClipboardOwner );
}





OUString SAL_CALL CWinClipboard::getName(  ) throw( RuntimeException )
{
    if ( rBHelper.bDisposed )
        throw DisposedException("object is already disposed",
                                 static_cast< XClipboardEx* >( this ) );

    if ( NULL != m_pImpl.get( ) )
        return m_pImpl->getName( );

    return OUString("");
}





void SAL_CALL CWinClipboard::flushClipboard( ) throw( RuntimeException )
{
    MutexGuard aGuard( m_aMutex );

    if ( rBHelper.bDisposed )
        throw DisposedException("object is already disposed",
                                 static_cast< XClipboardEx* >( this ) );

    if ( NULL != m_pImpl.get( ) )
        m_pImpl->flushClipboard( );
}





sal_Int8 SAL_CALL CWinClipboard::getRenderingCapabilities(  ) throw( RuntimeException )
{
    if ( rBHelper.bDisposed )
        throw DisposedException("object is already disposed",
                                 static_cast< XClipboardEx* >( this ) );

    if ( NULL != m_pImpl.get( ) )
        return m_pImpl->getRenderingCapabilities( );

    return 0;
}









void SAL_CALL CWinClipboard::addClipboardListener( const Reference< XClipboardListener >& listener )
    throw( RuntimeException )
{
    if ( rBHelper.bDisposed )
        throw DisposedException("object is already disposed",
                                 static_cast< XClipboardEx* >( this ) );

    
    if ( !listener.is( ) )
        throw IllegalArgumentException("empty reference",
                                        static_cast< XClipboardEx* >( this ),
                                        1 );

    rBHelper.aLC.addInterface( getCppuType( &listener ), listener );
}





void SAL_CALL CWinClipboard::removeClipboardListener( const Reference< XClipboardListener >& listener )
    throw( RuntimeException )
{
    if ( rBHelper.bDisposed )
        throw DisposedException("object is already disposed",
                                 static_cast< XClipboardEx* >( this ) );

    
    if ( !listener.is( ) )
        throw IllegalArgumentException("empty reference",
                                        static_cast< XClipboardEx* >( this ),
                                        1 );

    rBHelper.aLC.removeInterface( getCppuType( &listener ), listener );
}





void SAL_CALL CWinClipboard::notifyAllClipboardListener( )
{
    if ( !rBHelper.bDisposed )
    {
        ClearableMutexGuard aGuard( rBHelper.rMutex );
        if ( !rBHelper.bDisposed )
        {
            aGuard.clear( );

            OInterfaceContainerHelper* pICHelper = rBHelper.aLC.getContainer(
                getCppuType( ( Reference< XClipboardListener > * ) 0 ) );

            if ( pICHelper )
            {
                try
                {
                    OInterfaceIteratorHelper iter(*pICHelper);
                    Reference<XTransferable> rXTransf(m_pImpl->getContents());
                    ClipboardEvent aClipbEvent(static_cast<XClipboard*>(this), rXTransf);

                    while(iter.hasMoreElements())
                    {
                        try
                        {
                            Reference<XClipboardListener> xCBListener(iter.next(), UNO_QUERY);
                            if (xCBListener.is())
                                xCBListener->changedContents(aClipbEvent);
                        }
                        catch(RuntimeException&)
                        {
                            OSL_FAIL( "RuntimeException caught" );
                        }
                    }
                }
                catch(const ::com::sun::star::lang::DisposedException&)
                {
                    OSL_FAIL("Service Manager disposed");

                    
                    m_pImpl->unregisterClipboardViewer();
                }

            } 
        } 
    } 
}






void SAL_CALL CWinClipboard::disposing()
{
    
    m_pImpl->dispose( );

    
    m_pImpl.reset();
}





OUString SAL_CALL CWinClipboard::getImplementationName(  )
    throw(RuntimeException)
{
    return OUString( WINCLIPBOARD_IMPL_NAME );
}


sal_Bool SAL_CALL CWinClipboard::supportsService( const OUString& ServiceName )
    throw(RuntimeException)
{
    return cppu::supportsService(this, ServiceName);
}





Sequence< OUString > SAL_CALL CWinClipboard::getSupportedServiceNames(   )
    throw(RuntimeException)
{
    return WinClipboard_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
