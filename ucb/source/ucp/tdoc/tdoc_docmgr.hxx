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

#ifndef INCLUDED_TDOC_DOCMGR_HXX
#define INCLUDED_TDOC_DOCMGR_HXX

#include <map>

#include "osl/mutex.hxx"

#include "cppuhelper/implbase1.hxx"

#include "com/sun/star/document/XEventListener.hpp"
#include "com/sun/star/embed/XStorage.hpp"
#include "com/sun/star/frame/XModel.hpp"
#include "com/sun/star/frame/XModuleManager2.hpp"
#include "com/sun/star/frame/XGlobalEventBroadcaster.hpp"
#include "com/sun/star/util/XCloseListener.hpp"

namespace tdoc_ucp {

    class OfficeDocumentsEventListener
    {
    public:
        virtual void notifyDocumentOpened( const rtl::OUString & rDocId ) = 0;
        virtual void notifyDocumentClosed( const rtl::OUString & rDocId ) = 0;

    protected:
        ~OfficeDocumentsEventListener() {}
    };

    //=======================================================================

    struct StorageInfo
    {
        rtl::OUString aTitle;
        com::sun::star::uno::Reference<
            com::sun::star::embed::XStorage > xStorage;
        com::sun::star::uno::Reference<
            com::sun::star::frame::XModel >   xModel;

        StorageInfo() {}; // needed for STL map only.

        StorageInfo(
            const rtl::OUString & rTitle,
            const com::sun::star::uno::Reference<
                com::sun::star::embed::XStorage > & rxStorage,
            const com::sun::star::uno::Reference<
                com::sun::star::frame::XModel > & rxModel )
        : aTitle( rTitle ), xStorage( rxStorage ), xModel( rxModel ) {}
    };

    //=======================================================================

    struct ltref
    {
        bool operator()(
            const rtl::OUString & r1, const rtl::OUString & r2 ) const
        {
            return r1 < r2;
        }
    };

    typedef std::map< rtl::OUString, StorageInfo, ltref > DocumentList;

    //=======================================================================

    class OfficeDocumentsManager :
        public cppu::WeakImplHelper1< com::sun::star::document::XEventListener >
    {
        class OfficeDocumentsCloseListener :
           public cppu::WeakImplHelper1< com::sun::star::util::XCloseListener >

        {
        public:
            OfficeDocumentsCloseListener( OfficeDocumentsManager * pMgr )
            : m_pManager( pMgr ) {};

            // util::XCloseListener
            virtual void SAL_CALL queryClosing(
                    const ::com::sun::star::lang::EventObject& Source,
                    ::sal_Bool GetsOwnership )
                throw (::com::sun::star::util::CloseVetoException,
                       ::com::sun::star::uno::RuntimeException);

            virtual void SAL_CALL notifyClosing(
                    const ::com::sun::star::lang::EventObject& Source )
                throw (::com::sun::star::uno::RuntimeException);

            // lang::XEventListener (base of util::XCloseListener)
            virtual void SAL_CALL disposing(
                    const com::sun::star::lang::EventObject & Source )
                throw ( com::sun::star::uno::RuntimeException );
        private:
            OfficeDocumentsManager * m_pManager;
        };

    public:
        OfficeDocumentsManager(
            const com::sun::star::uno::Reference<
                com::sun::star::uno::XComponentContext > & rxContext,
            OfficeDocumentsEventListener * pDocEventListener );
        virtual ~OfficeDocumentsManager();

        void destroy();

        // document::XEventListener
        virtual void SAL_CALL notifyEvent(
                const com::sun::star::document::EventObject & Event )
            throw ( com::sun::star::uno::RuntimeException );

        // lang::XEventListener (base of document::XEventListener)
        virtual void SAL_CALL disposing(
                const com::sun::star::lang::EventObject & Source )
            throw ( com::sun::star::uno::RuntimeException );

        // Non-interface
        com::sun::star::uno::Reference< com::sun::star::embed::XStorage >
        queryStorage( const rtl::OUString & rDocId );

        rtl::OUString
        queryDocumentId(
            const com::sun::star::uno::Reference<
                com::sun::star::frame::XModel > & xModel );

        com::sun::star::uno::Reference< com::sun::star::frame::XModel >
        queryDocumentModel( const rtl::OUString & rDocId );

        com::sun::star::uno::Sequence< rtl::OUString >
        queryDocuments();

        rtl::OUString
        queryStorageTitle( const rtl::OUString & rDocId );

    private:
        void buildDocumentsList();

        bool
        isOfficeDocument(
            const com::sun::star::uno::Reference<
                com::sun::star::uno::XInterface > & xDoc );

        bool
        isDocumentPreview(
            const com::sun::star::uno::Reference<
                com::sun::star::frame::XModel > & xModel );

        bool
        isWithoutOrInTopLevelFrame(
            const com::sun::star::uno::Reference<
                com::sun::star::frame::XModel > & xModel );

        bool
        isBasicIDE(
            const com::sun::star::uno::Reference<
                com::sun::star::frame::XModel > & xModel );

        bool
        isHelpDocument(
            const com::sun::star::uno::Reference<
                com::sun::star::frame::XModel > & xModel );

        osl::Mutex                                          m_aMtx;
        com::sun::star::uno::Reference<
            com::sun::star::uno::XComponentContext >        m_xContext;
        com::sun::star::uno::Reference<
            com::sun::star::frame::XGlobalEventBroadcaster > m_xDocEvtNotifier;
        com::sun::star::uno::Reference<
            com::sun::star::frame::XModuleManager2 >        m_xModuleMgr;
        DocumentList                                        m_aDocs;
        OfficeDocumentsEventListener *                      m_pDocEventListener;
        com::sun::star::uno::Reference<
            com::sun::star::util::XCloseListener >          m_xDocCloseListener;
    };

} // namespace tdoc_ucp

#endif /* !INCLUDED_TDOC_DOCMGR_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
