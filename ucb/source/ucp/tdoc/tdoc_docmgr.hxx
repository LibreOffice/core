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

#ifndef INCLUDED_UCB_SOURCE_UCP_TDOC_TDOC_DOCMGR_HXX
#define INCLUDED_UCB_SOURCE_UCP_TDOC_TDOC_DOCMGR_HXX

#include <map>

#include <rtl/ref.hxx>
#include "osl/mutex.hxx"

#include <cppuhelper/implbase.hxx>

#include "com/sun/star/document/XDocumentEventListener.hpp"
#include "com/sun/star/embed/XStorage.hpp"
#include "com/sun/star/frame/XModel.hpp"
#include "com/sun/star/frame/XModuleManager2.hpp"
#include "com/sun/star/frame/XGlobalEventBroadcaster.hpp"
#include "com/sun/star/util/XCloseListener.hpp"

namespace tdoc_ucp {

    class ContentProvider;

    struct StorageInfo
    {
        OUString aTitle;
        css::uno::Reference< css::embed::XStorage > xStorage;
        css::uno::Reference< css::frame::XModel >   xModel;

        StorageInfo() {}; // needed for STL map only.

        StorageInfo(
            const OUString & rTitle,
            const css::uno::Reference< css::embed::XStorage > & rxStorage,
            const css::uno::Reference< css::frame::XModel > & rxModel )
        : aTitle( rTitle ), xStorage( rxStorage ), xModel( rxModel ) {}
    };


    struct ltref
    {
        bool operator()(
            const OUString & r1, const OUString & r2 ) const
        {
            return r1 < r2;
        }
    };

    typedef std::map< OUString, StorageInfo, ltref > DocumentList;


    class OfficeDocumentsManager :
        public cppu::WeakImplHelper< css::document::XDocumentEventListener >
    {
        class OfficeDocumentsCloseListener :
           public cppu::WeakImplHelper< css::util::XCloseListener >

        {
        public:
            explicit OfficeDocumentsCloseListener( OfficeDocumentsManager * pMgr )
                : m_pManager( pMgr ) {}

            // util::XCloseListener
            virtual void SAL_CALL queryClosing(
                    const css::lang::EventObject& Source,
                    sal_Bool GetsOwnership )
                throw (css::util::CloseVetoException,
                       css::uno::RuntimeException, std::exception) override;

            virtual void SAL_CALL notifyClosing(
                    const css::lang::EventObject& Source )
                throw (css::uno::RuntimeException, std::exception) override;

            // lang::XEventListener (base of util::XCloseListener)
            virtual void SAL_CALL disposing(
                    const css::lang::EventObject & Source )
                throw ( css::uno::RuntimeException, std::exception ) override;

            void Dispose() { m_pManager = nullptr; }

        private:
            OfficeDocumentsManager * m_pManager;
        };

    public:
        OfficeDocumentsManager(
            const css::uno::Reference< css::uno::XComponentContext > & rxContext,
            ContentProvider * pDocEventListener );
        virtual ~OfficeDocumentsManager();

        void destroy();

        // document::XDocumentEventListener
        virtual void SAL_CALL documentEventOccured(
                const css::document::DocumentEvent & Event )
            throw ( css::uno::RuntimeException, std::exception ) override;

        // lang::XEventListener (base of document::XDocumentEventListener)
        virtual void SAL_CALL disposing(
                const css::lang::EventObject & Source )
            throw ( css::uno::RuntimeException, std::exception ) override;

        // Non-interface
        css::uno::Reference< css::embed::XStorage >
        queryStorage( const OUString & rDocId );

        static OUString queryDocumentId(
            const css::uno::Reference< css::frame::XModel > & xModel );

        css::uno::Reference< css::frame::XModel >
        queryDocumentModel( const OUString & rDocId );

        css::uno::Sequence< OUString >
        queryDocuments();

        OUString
        queryStorageTitle( const OUString & rDocId );

    private:
        void buildDocumentsList();

        bool isOfficeDocument(
            const css::uno::Reference< css::uno::XInterface > & xDoc );

        static bool isDocumentPreview(
            const css::uno::Reference< css::frame::XModel > & xModel );

        static bool isWithoutOrInTopLevelFrame(
            const css::uno::Reference< css::frame::XModel > & xModel );

        bool
        isBasicIDE(
            const css::uno::Reference< css::frame::XModel > & xModel );

        static bool isHelpDocument(
            const css::uno::Reference< css::frame::XModel > & xModel );

        osl::Mutex                                          m_aMtx;
        css::uno::Reference< css::uno::XComponentContext >         m_xContext;
        css::uno::Reference< css::frame::XGlobalEventBroadcaster > m_xDocEvtNotifier;
        css::uno::Reference< css::frame::XModuleManager2 >         m_xModuleMgr;
        DocumentList                                        m_aDocs;
        ContentProvider * const                             m_pDocEventListener;
        ::rtl::Reference<OfficeDocumentsCloseListener> const m_xDocCloseListener;
    };

} // namespace tdoc_ucp

#endif // INCLUDED_UCB_SOURCE_UCP_TDOC_TDOC_DOCMGR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
