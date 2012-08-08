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


#ifndef _WINCLIPBOARD_HXX_
#define _WINCLIPBOARD_HXX_

#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <cppuhelper/compbase4.hxx>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardEx.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardOwner.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardListener.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardNotifier.hpp>
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <osl/conditn.hxx>

#include <memory>

// forward
class CWinClipbImpl;

//------------------------------------------------------------------------
// implements the XClipboard[Ex] ... interfaces
// for the clipboard viewer mechanism we need a static callback function
// and a static member to reasocciate from this static function to the
// class instance
// watch out: we are using only one static member variable and not a list
// because we assume to be instanciated only once
// this will be asured by an OneInstanceFactory of the service and not
// by this class!
//------------------------------------------------------------------------

// helper class, so that the mutex is constructed
// before the constructor of WeakComponentImplHelper
// will be called and initialized with this mutex
class CWinClipboardDummy
{
protected:
    osl::Mutex      m_aMutex;
    osl::Mutex      m_aCbListenerMutex;
};

class CWinClipboard :
    public CWinClipboardDummy,
    public cppu::WeakComponentImplHelper4<
        ::com::sun::star::datatransfer::clipboard::XClipboardEx, \
        ::com::sun::star::datatransfer::clipboard::XFlushableClipboard,
        ::com::sun::star::datatransfer::clipboard::XClipboardNotifier,
        ::com::sun::star::lang::XServiceInfo >
{
public:
    CWinClipboard( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rServiceManager,
                   const ::rtl::OUString& aClipboardName );

    //------------------------------------------------
    // XClipboard
    //------------------------------------------------

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > SAL_CALL getContents(  )
        throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setContents(
        const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& xTransferable,
        const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardOwner >& xClipboardOwner )
        throw( ::com::sun::star::uno::RuntimeException );

    virtual ::rtl::OUString SAL_CALL getName(  )
        throw( ::com::sun::star::uno::RuntimeException );

    //------------------------------------------------
    // XFlushableClipboard
    //------------------------------------------------

    virtual void SAL_CALL flushClipboard( ) throw( com::sun::star::uno::RuntimeException );

    //------------------------------------------------
    // XClipboardEx
    //------------------------------------------------

    virtual sal_Int8 SAL_CALL getRenderingCapabilities(  ) throw( ::com::sun::star::uno::RuntimeException );

    //------------------------------------------------
    // XClipboardNotifier
    //------------------------------------------------

    virtual void SAL_CALL addClipboardListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardListener >& listener )
        throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removeClipboardListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardListener >& listener )
        throw( ::com::sun::star::uno::RuntimeException );

    //------------------------------------------------
    // overwrite base class method, which is called
    // by base class dispose function
    //------------------------------------------------

    virtual void SAL_CALL disposing();

    //------------------------------------------------
    // XServiceInfo
    //------------------------------------------------

    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
        throw(::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
        throw(::com::sun::star::uno::RuntimeException);

private:
    void SAL_CALL notifyAllClipboardListener( );

private:
    ::std::auto_ptr< CWinClipbImpl >                                                  m_pImpl;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >  m_SrvMgr;

    friend class CWinClipbImpl;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
