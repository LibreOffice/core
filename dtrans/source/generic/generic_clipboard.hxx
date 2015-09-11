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

#ifndef INCLUDED_DTRANS_SOURCE_GENERIC_GENERIC_CLIPBOARD_HXX
#define INCLUDED_DTRANS_SOURCE_GENERIC_GENERIC_CLIPBOARD_HXX

#include <cppuhelper/compbase.hxx>

#include <com/sun/star/datatransfer/clipboard/XClipboardEx.hpp>

#include <com/sun/star/datatransfer/clipboard/XClipboardNotifier.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>

#define GENERIC_CLIPBOARD_IMPLEMENTATION_NAME "com.sun.star.comp.datatransfer.clipboard.GenericClipboard"

namespace dtrans
{

    class GenericClipboard : public ::cppu::WeakComponentImplHelper <
        css::datatransfer::clipboard::XClipboardEx,
        css::datatransfer::clipboard::XClipboardNotifier,
        css::lang::XServiceInfo,
        css::lang::XInitialization >
    {
        ::osl::Mutex m_aMutex;
        OUString     m_aName;

        css::uno::Reference< css::datatransfer::XTransferable > m_aContents;
        css::uno::Reference< css::datatransfer::clipboard::XClipboardOwner > m_aOwner;

        sal_Bool m_bInitialized;
        virtual ~GenericClipboard();

    public:

        GenericClipboard();

        /*
         * XInitialization
         */

        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
            throw(css::uno::Exception, css::uno::RuntimeException);

        /*
         * XServiceInfo
         */

        virtual OUString SAL_CALL getImplementationName(  )
            throw(css::uno::RuntimeException);

        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw(css::uno::RuntimeException);

        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
            throw(css::uno::RuntimeException);

        /*
         * XClipboard
         */

        virtual css::uno::Reference< css::datatransfer::XTransferable > SAL_CALL getContents()
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL setContents(
            const css::uno::Reference< css::datatransfer::XTransferable >& xTrans,
            const css::uno::Reference< css::datatransfer::clipboard::XClipboardOwner >& xClipboardOwner )
            throw(css::uno::RuntimeException);

        virtual OUString SAL_CALL getName()
            throw(css::uno::RuntimeException);

        /*
         * XClipboardEx
         */

        virtual sal_Int8 SAL_CALL getRenderingCapabilities()
            throw(css::uno::RuntimeException);

        /*
         * XClipboardNotifier
         */

        virtual void SAL_CALL addClipboardListener(
            const css::uno::Reference< css::datatransfer::clipboard::XClipboardListener >& listener )
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL removeClipboardListener(
            const css::uno::Reference< css::datatransfer::clipboard::XClipboardListener >& listener )
            throw(css::uno::RuntimeException);

    };

}

css::uno::Sequence< OUString > SAL_CALL GenericClipboard_getSupportedServiceNames();
css::uno::Reference< css::uno::XInterface > SAL_CALL GenericClipboard_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & xMultiServiceFactory);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
