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

#include <osl/diagnose.h>
#include "WinClipboard.hxx"
#include <com/sun/star/datatransfer/clipboard/ClipboardEvent.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/ref.hxx>
#include "WinClipbImpl.hxx"

using namespace osl;
using namespace std;
using namespace cppu;

using namespace com::sun::star::uno;
using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::clipboard;
using namespace com::sun::star::lang;

/*XEventListener,*/
CWinClipboard::CWinClipboard( const Reference< XComponentContext >& rxContext, const OUString& aClipboardName ) :
    WeakComponentImplHelper< XSystemClipboard, XFlushableClipboard, XServiceInfo >( m_aCbListenerMutex ),
    m_xContext( rxContext )
{
    m_pImpl.reset( new CWinClipbImpl( aClipboardName, this ) );
}

// XClipboard

// to avoid unnecessary traffic we check first if there is a clipboard
// content which was set via setContent, in this case we don't need
// to query the content from the clipboard, create a new wrapper object
// and so on, we simply return the original XTransferable instead of our
// DOTransferable

Reference< XTransferable > SAL_CALL CWinClipboard::getContents( )
{
    MutexGuard aGuard( m_aMutex );

    if ( rBHelper.bDisposed )
        throw DisposedException("object is already disposed",
                                 static_cast< XClipboardEx* >( this ) );

    if ( m_pImpl )
        return m_pImpl->getContents( );

    return Reference< XTransferable >( );
}

void SAL_CALL CWinClipboard::setContents( const Reference< XTransferable >& xTransferable,
                                          const Reference< XClipboardOwner >& xClipboardOwner )
{
    MutexGuard aGuard( m_aMutex );

    if ( rBHelper.bDisposed )
        throw DisposedException("object is already disposed",
                                 static_cast< XClipboardEx* >( this ) );

    if ( m_pImpl )
        m_pImpl->setContents( xTransferable, xClipboardOwner );
}

OUString SAL_CALL CWinClipboard::getName(  )
{
    if ( rBHelper.bDisposed )
        throw DisposedException("object is already disposed",
                                 static_cast< XClipboardEx* >( this ) );

    if ( m_pImpl )
        return m_pImpl->getName( );

    return OUString();
}

// XFlushableClipboard

void SAL_CALL CWinClipboard::flushClipboard( )
{
    MutexGuard aGuard( m_aMutex );

    if ( rBHelper.bDisposed )
        throw DisposedException("object is already disposed",
                                 static_cast< XClipboardEx* >( this ) );

    if ( m_pImpl )
        m_pImpl->flushClipboard( );
}

// XClipboardEx

sal_Int8 SAL_CALL CWinClipboard::getRenderingCapabilities(  )
{
    if ( rBHelper.bDisposed )
        throw DisposedException("object is already disposed",
                                 static_cast< XClipboardEx* >( this ) );

    if ( m_pImpl )
        return CWinClipbImpl::getRenderingCapabilities( );

    return 0;
}

// XClipboardNotifier

void SAL_CALL CWinClipboard::addClipboardListener( const Reference< XClipboardListener >& listener )
{
    if ( rBHelper.bDisposed )
        throw DisposedException("object is already disposed",
                                 static_cast< XClipboardEx* >( this ) );

    // check input parameter
    if ( !listener.is( ) )
        throw IllegalArgumentException("empty reference",
                                        static_cast< XClipboardEx* >( this ),
                                        1 );

    rBHelper.aLC.addInterface( cppu::UnoType<decltype(listener)>::get(), listener );
}

void SAL_CALL CWinClipboard::removeClipboardListener( const Reference< XClipboardListener >& listener )
{
    if ( rBHelper.bDisposed )
        throw DisposedException("object is already disposed",
                                 static_cast< XClipboardEx* >( this ) );

    // check input parameter
    if ( !listener.is( ) )
        throw IllegalArgumentException("empty reference",
                                        static_cast< XClipboardEx* >( this ),
                                        1 );

    rBHelper.aLC.removeInterface( cppu::UnoType<decltype(listener)>::get(), listener );
}

void CWinClipboard::notifyAllClipboardListener( )
{
    if ( !rBHelper.bDisposed )
    {
        ClearableMutexGuard aGuard( rBHelper.rMutex );
        if ( !rBHelper.bDisposed )
        {
            aGuard.clear( );

            OInterfaceContainerHelper* pICHelper = rBHelper.aLC.getContainer(
                cppu::UnoType<XClipboardListener>::get());

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
                catch(const css::lang::DisposedException&)
                {
                    OSL_FAIL("Service Manager disposed");

                    // no further clipboard changed notifications
                    m_pImpl->unregisterClipboardViewer();
                }

            } // end if
        } // end if
    } // end if
}

// overwritten base class method which will be
// called by the base class dispose method

void SAL_CALL CWinClipboard::disposing()
{
    // do my own stuff
    m_pImpl->dispose( );

    // force destruction of the impl class
    m_pImpl.reset();
}

// XServiceInfo

OUString SAL_CALL CWinClipboard::getImplementationName(  )
{
    return "com.sun.star.datatransfer.clipboard.ClipboardW32";
}

sal_Bool SAL_CALL CWinClipboard::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL CWinClipboard::getSupportedServiceNames(   )
{
    return { "com.sun.star.datatransfer.clipboard.SystemClipboard" };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
dtrans_CWinClipboard_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    static rtl::Reference<CWinClipboard> g_Instance(new CWinClipboard(context, ""));
    g_Instance->acquire();
    return static_cast<cppu::OWeakObject*>(g_Instance.get());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
