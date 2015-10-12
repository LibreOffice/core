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
#include <cppuhelper/compbase.hxx>

#define X11_CLIPBOARD_IMPLEMENTATION_NAME "com.sun.star.datatransfer.X11ClipboardSupport"

namespace x11 {

    class X11Clipboard :
        public ::cppu::WeakComponentImplHelper <
        css::datatransfer::clipboard::XSystemClipboard,
        css::lang::XServiceInfo
        >,
        public SelectionAdaptor
    {
        css::uno::Reference< css::datatransfer::XTransferable > m_aContents;
        css::uno::Reference< css::datatransfer::clipboard::XClipboardOwner > m_aOwner;

        SelectionManager&                                       m_rSelectionManager;
        css::uno::Reference< css::lang::XInitialization >   m_xSelectionManager;
        ::std::list< css::uno::Reference< css::datatransfer::clipboard::XClipboardListener > > m_aListeners;
        Atom                                                    m_aSelection;

        X11Clipboard( SelectionManager& rManager, Atom aSelection );

    protected:

        friend class SelectionManager;

        void fireChangedContentsEvent();
        void clearContents();

    public:

        static css::uno::Reference<css::datatransfer::clipboard::XClipboard>
        create( SelectionManager& rManager, Atom aSelection );

        virtual ~X11Clipboard();

        /*
         * XServiceInfo
         */

        virtual OUString SAL_CALL getImplementationName(  )
            throw(css::uno::RuntimeException, std::exception) override;

        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw(css::uno::RuntimeException, std::exception) override;

        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
            throw(css::uno::RuntimeException, std::exception) override;

        /*
         * XClipboard
         */

        virtual css::uno::Reference< css::datatransfer::XTransferable > SAL_CALL getContents()
            throw(css::uno::RuntimeException, std::exception) override;

        virtual void SAL_CALL setContents(
            const css::uno::Reference< css::datatransfer::XTransferable >& xTrans,
            const css::uno::Reference< css::datatransfer::clipboard::XClipboardOwner >& xClipboardOwner )
            throw(css::uno::RuntimeException, std::exception) override;

        virtual OUString SAL_CALL getName()
            throw(css::uno::RuntimeException, std::exception) override;

        /*
         * XClipboardEx
         */

        virtual sal_Int8 SAL_CALL getRenderingCapabilities()
            throw(css::uno::RuntimeException, std::exception) override;

        /*
         * XClipboardNotifier
         */
        virtual void SAL_CALL addClipboardListener(
            const css::uno::Reference< css::datatransfer::clipboard::XClipboardListener >& listener )
            throw(css::uno::RuntimeException, std::exception) override;

        virtual void SAL_CALL removeClipboardListener(
            const css::uno::Reference< css::datatransfer::clipboard::XClipboardListener >& listener )
            throw(css::uno::RuntimeException, std::exception) override;

        /*
         *  SelectionAdaptor
         */
        virtual css::uno::Reference< css::datatransfer::XTransferable > getTransferable() override;
        virtual void clearTransferable() override;
        virtual void fireContentsChanged() override;
        virtual css::uno::Reference< css::uno::XInterface > getReference() throw() override;
    };

    css::uno::Sequence< OUString > SAL_CALL X11Clipboard_getSupportedServiceNames();
    css::uno::Reference< css::uno::XInterface > SAL_CALL X11Clipboard_createInstance(
        const css::uno::Reference< css::lang::XMultiServiceFactory > & xMultiServiceFactory);

} // namepspace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
