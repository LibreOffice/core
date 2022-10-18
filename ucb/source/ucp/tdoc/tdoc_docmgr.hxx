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

#pragma once

#include <rtl/ref.hxx>

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/document/XDocumentEventListener.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XModuleManager2.hpp>
#include <com/sun/star/frame/XGlobalEventBroadcaster.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/XCloseListener.hpp>

#include <map>
#include <mutex>
#include <unordered_map>
#include <utility>

namespace tdoc_ucp {

    class ContentProvider;

    struct StorageInfo
    {
        OUString aTitle;
        css::uno::Reference< css::embed::XStorage > xStorage;
        css::uno::Reference< css::frame::XModel >   xModel;
        std::unordered_map<OUString, css::util::DateTime> streamDateModified;

        StorageInfo() {}; // needed for STL map only.

        StorageInfo(
            OUString _aTitle,
            css::uno::Reference< css::embed::XStorage > _xStorage,
            css::uno::Reference< css::frame::XModel > _xModel )
        : aTitle(std::move( _aTitle )), xStorage(std::move( _xStorage )), xModel(std::move( _xModel )) {}
    };


    typedef std::map< OUString, StorageInfo > DocumentList;


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
                    sal_Bool GetsOwnership ) override;

            virtual void SAL_CALL notifyClosing(
                    const css::lang::EventObject& Source ) override;

            // lang::XEventListener (base of util::XCloseListener)
            virtual void SAL_CALL disposing(
                    const css::lang::EventObject & Source ) override;

            void Dispose() { m_pManager = nullptr; }

        private:
            OfficeDocumentsManager * m_pManager;
        };

    public:
        OfficeDocumentsManager(
            const css::uno::Reference< css::uno::XComponentContext > & rxContext,
            ContentProvider * pDocEventListener );
        virtual ~OfficeDocumentsManager() override;

        void destroy();

        // document::XDocumentEventListener
        virtual void SAL_CALL documentEventOccured(
                const css::document::DocumentEvent & Event ) override;

        // lang::XEventListener (base of document::XDocumentEventListener)
        virtual void SAL_CALL disposing(
                const css::lang::EventObject & Source ) override;

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

        css::util::DateTime queryStreamDateModified(OUString const & uri);

        void updateStreamDateModified(OUString const & uri);

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

        std::mutex                                          m_aMtx;
        css::uno::Reference< css::uno::XComponentContext >         m_xContext;
        css::uno::Reference< css::frame::XGlobalEventBroadcaster > m_xDocEvtNotifier;
        css::uno::Reference< css::frame::XModuleManager2 >         m_xModuleMgr;
        DocumentList                                        m_aDocs;
        ContentProvider * const                             m_pDocEventListener;
        ::rtl::Reference<OfficeDocumentsCloseListener> const m_xDocCloseListener;
    };

} // namespace tdoc_ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
