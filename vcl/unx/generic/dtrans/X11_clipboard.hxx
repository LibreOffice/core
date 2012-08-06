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

#ifndef _DTRANS_X11_CLIPBOARD_HXX_
#define _DTRANS_X11_CLIPBOARD_HXX_

#include <X11_selection.hxx>

#include <com/sun/star/datatransfer/clipboard/XClipboardEx.hpp>

#include <com/sun/star/datatransfer/clipboard/XClipboardNotifier.hpp>
#include <cppuhelper/compbase4.hxx>

// ------------------------------------------------------------------------

#define X11_CLIPBOARD_IMPLEMENTATION_NAME "com.sun.star.datatransfer.X11ClipboardSupport"

namespace x11 {

    class X11Clipboard :
        public ::cppu::WeakComponentImplHelper4 <
        ::com::sun::star::datatransfer::clipboard::XClipboardEx,
        ::com::sun::star::datatransfer::clipboard::XClipboardNotifier,
        ::com::sun::star::lang::XServiceInfo,
        ::com::sun::star::lang::XInitialization
        >,
        public SelectionAdaptor
    {
        com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > m_aContents;
        com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardOwner > m_aOwner;

        SelectionManager&                                       m_rSelectionManager;
        com::sun::star::uno::Reference< ::com::sun::star::lang::XInitialization >   m_xSelectionManager;
        ::std::list< com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardListener > > m_aListeners;
        Atom                                                    m_aSelection;

    protected:


        friend class SelectionManager;

        void fireChangedContentsEvent();
        void clearContents();

    public:

        X11Clipboard( SelectionManager& rManager, Atom aSelection );
        virtual ~X11Clipboard();

        static X11Clipboard* get( const ::rtl::OUString& rDisplayName, Atom aSelection );

        /*
         *  XInitialization
         */
        virtual void SAL_CALL initialize( const Sequence< Any >& arguments ) throw(  ::com::sun::star::uno::Exception );

        /*
         * XServiceInfo
         */

        virtual ::rtl::OUString SAL_CALL getImplementationName(  )
            throw(RuntimeException);

        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
            throw(RuntimeException);

        virtual Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
            throw(RuntimeException);

        /*
         * XClipboard
         */

        virtual com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > SAL_CALL getContents()
            throw(RuntimeException);

        virtual void SAL_CALL setContents(
            const com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& xTrans,
            const com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardOwner >& xClipboardOwner )
            throw(RuntimeException);

        virtual ::rtl::OUString SAL_CALL getName()
            throw(RuntimeException);

        /*
         * XClipboardEx
         */

        virtual sal_Int8 SAL_CALL getRenderingCapabilities()
            throw(RuntimeException);

        /*
         * XClipboardNotifier
         */
        virtual void SAL_CALL addClipboardListener(
            const com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardListener >& listener )
            throw(RuntimeException);

        virtual void SAL_CALL removeClipboardListener(
            const com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardListener >& listener )
            throw(RuntimeException);

        /*
         *  SelectionAdaptor
         */
        virtual com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > getTransferable();
        virtual void clearTransferable();
        virtual void fireContentsChanged();
        virtual com::sun::star::uno::Reference< XInterface > getReference() throw();
    };

// ------------------------------------------------------------------------

    Sequence< ::rtl::OUString > SAL_CALL X11Clipboard_getSupportedServiceNames();
    com::sun::star::uno::Reference< XInterface > SAL_CALL X11Clipboard_createInstance(
        const com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & xMultiServiceFactory);

// ------------------------------------------------------------------------

} // namepspace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
