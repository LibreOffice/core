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

#ifndef _DTRANS_CLIPBOARDMANAGER_HXX_
#define _DTRANS_CLIPBOARDMANAGER_HXX_

#include <cppuhelper/compbase3.hxx>

#include <com/sun/star/datatransfer/clipboard/XClipboardManager.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <map>

// ------------------------------------------------------------------------

#define CLIPBOARDMANAGER_IMPLEMENTATION_NAME "com.sun.star.comp.datatransfer.ClipboardManager"

// ------------------------------------------------------------------------

typedef ::std::map< OUString, ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard > > ClipboardMap;

// ------------------------------------------------------------------------

namespace dtrans
{

    class ClipboardManager : public ::cppu::WeakComponentImplHelper3 <
      ::com::sun::star::datatransfer::clipboard::XClipboardManager,
      ::com::sun::star::lang::XEventListener,
      ::com::sun::star::lang::XServiceInfo >
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
            throw(::com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw(::com::sun::star::uno::RuntimeException);

        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
            throw(::com::sun::star::uno::RuntimeException);

        /*
         * XComponent
         */

        virtual void SAL_CALL dispose()
            throw(::com::sun::star::uno::RuntimeException);

        /*
         * XEventListener
         */

        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source )
            throw(::com::sun::star::uno::RuntimeException);

        /*
         * XClipboardManager
         */

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard > SAL_CALL getClipboard( const OUString& aName )
            throw(::com::sun::star::container::NoSuchElementException,
                  ::com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL addClipboard( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard >& xClipboard )
            throw(::com::sun::star::lang::IllegalArgumentException,
                  ::com::sun::star::container::ElementExistException,
                  ::com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL removeClipboard( const OUString& aName )
            throw(::com::sun::star::uno::RuntimeException);

        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL listClipboardNames(  )
            throw(::com::sun::star::uno::RuntimeException);


    };

}

// ------------------------------------------------------------------------

::com::sun::star::uno::Sequence< OUString > SAL_CALL ClipboardManager_getSupportedServiceNames();
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL ClipboardManager_createInstance(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & xMultiServiceFactory);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
