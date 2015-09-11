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

#ifndef INCLUDED_DTRANS_SOURCE_GENERIC_CLIPBOARDMANAGER_HXX
#define INCLUDED_DTRANS_SOURCE_GENERIC_CLIPBOARDMANAGER_HXX

#include <cppuhelper/compbase.hxx>

#include <com/sun/star/datatransfer/clipboard/XClipboardManager.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <map>

#define CLIPBOARDMANAGER_IMPLEMENTATION_NAME "com.sun.star.comp.datatransfer.ClipboardManager"

typedef ::std::map< OUString, css::uno::Reference< css::datatransfer::clipboard::XClipboard > > ClipboardMap;

namespace dtrans
{

    class ClipboardManager : public ::cppu::WeakComponentImplHelper <
      css::datatransfer::clipboard::XClipboardManager,
      css::lang::XEventListener,
      css::lang::XServiceInfo >
    {
        ClipboardMap m_aClipboardMap;
        ::osl::Mutex m_aMutex;

        const OUString m_aDefaultName;

        virtual ~ClipboardManager();
    protected:
        using WeakComponentImplHelperBase::disposing;
    public:

        ClipboardManager();

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
         * XComponent
         */

        virtual void SAL_CALL dispose()
            throw(css::uno::RuntimeException);

        /*
         * XEventListener
         */

        virtual void SAL_CALL disposing( const css::lang::EventObject& Source )
            throw(css::uno::RuntimeException);

        /*
         * XClipboardManager
         */

        virtual css::uno::Reference< css::datatransfer::clipboard::XClipboard > SAL_CALL getClipboard( const OUString& aName )
            throw(css::container::NoSuchElementException,
                  css::uno::RuntimeException);

        virtual void SAL_CALL addClipboard( const css::uno::Reference< css::datatransfer::clipboard::XClipboard >& xClipboard )
            throw(css::lang::IllegalArgumentException,
                  css::container::ElementExistException,
                  css::uno::RuntimeException);

        virtual void SAL_CALL removeClipboard( const OUString& aName )
            throw(css::uno::RuntimeException);

        virtual css::uno::Sequence< OUString > SAL_CALL listClipboardNames(  )
            throw(css::uno::RuntimeException);

    };

}

css::uno::Sequence< OUString > SAL_CALL ClipboardManager_getSupportedServiceNames();
css::uno::Reference< css::uno::XInterface > SAL_CALL ClipboardManager_createInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory > & xMultiServiceFactory);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
