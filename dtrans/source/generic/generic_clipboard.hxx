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

#ifndef _DTRANS_GENERIC_CLIPBOARD_HXX_
#define _DTRANS_GENERIC_CLIPBOARD_HXX_

#include <cppuhelper/compbase4.hxx>

#include <com/sun/star/datatransfer/clipboard/XClipboardEx.hpp>

#include <com/sun/star/datatransfer/clipboard/XClipboardNotifier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>

#define GENERIC_CLIPBOARD_IMPLEMENTATION_NAME "com.sun.star.comp.datatransfer.clipboard.GenericClipboard"

namespace dtrans
{

    class GenericClipboard : public ::cppu::WeakComponentImplHelper4 < \
    ::com::sun::star::datatransfer::clipboard::XClipboardEx, \
    ::com::sun::star::datatransfer::clipboard::XClipboardNotifier, \
    ::com::sun::star::lang::XServiceInfo, \
    ::com::sun::star::lang::XInitialization >
    {
        ::osl::Mutex m_aMutex;
        OUString m_aName;

        ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > m_aContents;
        ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardOwner > m_aOwner;

        sal_Bool m_bInitialized;
        virtual ~GenericClipboard();

    public:

        GenericClipboard();

        /*
         * XInitialization
         */

        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
            throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        /*
         * XServiceInfo
         */

        virtual OUString SAL_CALL getImplementationName(  )
            throw(::com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw(::com::sun::star::uno::RuntimeException);

        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
            throw(::com::sun::star::uno::RuntimeException);

        /*
         * XClipboard
         */

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > SAL_CALL getContents()
            throw(::com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL setContents(
            const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& xTrans,
            const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardOwner >& xClipboardOwner )
            throw(::com::sun::star::uno::RuntimeException);

        virtual OUString SAL_CALL getName()
            throw(::com::sun::star::uno::RuntimeException);

        /*
         * XClipboardEx
         */

        virtual sal_Int8 SAL_CALL getRenderingCapabilities()
            throw(::com::sun::star::uno::RuntimeException);

        /*
         * XClipboardNotifier
         */

        virtual void SAL_CALL addClipboardListener(
            const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardListener >& listener )
            throw(::com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL removeClipboardListener(
            const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardListener >& listener )
            throw(::com::sun::star::uno::RuntimeException);

    };

}

::com::sun::star::uno::Sequence< OUString > SAL_CALL GenericClipboard_getSupportedServiceNames();
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL GenericClipboard_createInstance(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & xMultiServiceFactory);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
