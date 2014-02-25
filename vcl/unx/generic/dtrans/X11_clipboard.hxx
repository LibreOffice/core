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

#ifndef INCLUDED_VCL_UNX_GENERIC_DTRANS_X11_CLIPBOARD_HXX
#define INCLUDED_VCL_UNX_GENERIC_DTRANS_X11_CLIPBOARD_HXX

#include <X11_selection.hxx>

#include <com/sun/star/datatransfer/clipboard/XClipboardEx.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardNotifier.hpp>
#include <com/sun/star/datatransfer/clipboard/XSystemClipboard.hpp>
#include <cppuhelper/compbase2.hxx>



#define X11_CLIPBOARD_IMPLEMENTATION_NAME "com.sun.star.datatransfer.X11ClipboardSupport"

namespace x11 {

    class X11Clipboard :
        public ::cppu::WeakComponentImplHelper2 <
        ::com::sun::star::datatransfer::clipboard::XSystemClipboard,
        ::com::sun::star::lang::XServiceInfo
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

        static X11Clipboard* get( const OUString& rDisplayName, Atom aSelection );

        /*
         * XServiceInfo
         */

        virtual OUString SAL_CALL getImplementationName(  )
            throw(RuntimeException, std::exception);

        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw(RuntimeException, std::exception);

        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
            throw(RuntimeException, std::exception);

        /*
         * XClipboard
         */

        virtual com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > SAL_CALL getContents()
            throw(RuntimeException, std::exception);

        virtual void SAL_CALL setContents(
            const com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& xTrans,
            const com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardOwner >& xClipboardOwner )
            throw(RuntimeException, std::exception);

        virtual OUString SAL_CALL getName()
            throw(RuntimeException, std::exception);

        /*
         * XClipboardEx
         */

        virtual sal_Int8 SAL_CALL getRenderingCapabilities()
            throw(RuntimeException, std::exception);

        /*
         * XClipboardNotifier
         */
        virtual void SAL_CALL addClipboardListener(
            const com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardListener >& listener )
            throw(RuntimeException, std::exception);

        virtual void SAL_CALL removeClipboardListener(
            const com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardListener >& listener )
            throw(RuntimeException, std::exception);

        /*
         *  SelectionAdaptor
         */
        virtual com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > getTransferable();
        virtual void clearTransferable();
        virtual void fireContentsChanged();
        virtual com::sun::star::uno::Reference< XInterface > getReference() throw();
    };



    Sequence< OUString > SAL_CALL X11Clipboard_getSupportedServiceNames();
    com::sun::star::uno::Reference< XInterface > SAL_CALL X11Clipboard_createInstance(
        const com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & xMultiServiceFactory);



} // namepspace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
