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

#ifndef INCLUDED_DBACCESS_SOURCE_CORE_DATAACCESS_DOCUMENTEVENTNOTIFIER_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_DATAACCESS_DOCUMENTEVENTNOTIFIER_HXX

#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/document/XDocumentEventListener.hpp>

#include <rtl/ref.hxx>

namespace cppu
{
    class OWeakObject;
}

namespace dbaccess
{

    class DocumentEventNotifier_Impl;
    // DocumentEventNotifier
    class DocumentEventNotifier
    {
    public:
        DocumentEventNotifier( ::cppu::OWeakObject& _rBroadcasterDocument, ::osl::Mutex& _rMutex );
        ~DocumentEventNotifier();

        void addLegacyEventListener( const css::uno::Reference< css::document::XEventListener >& _Listener );
        void removeLegacyEventListener( const css::uno::Reference< css::document::XEventListener >& _Listener );
        void addDocumentEventListener( const css::uno::Reference< css::document::XDocumentEventListener >& _Listener );
        void removeDocumentEventListener( const css::uno::Reference< css::document::XDocumentEventListener >& _Listener );

        /** disposes the instance
            @precond
                the mutex is not locked
        */
        void    disposing();

        /** tells the instance that its document is completely initialized now.

            Before you call this method, no notification will actually happen

            @precond
                the mutex is locked
        */
        void    onDocumentInitialized();

        /** notifies a document event described by the given parameters

            @precond
                the mutex is not locked
            @precond
                ->onDocumentInitialized has been called
        */
        void    notifyDocumentEvent(
                    const OUString& _EventName,
                    const css::uno::Reference< css::frame::XController2 >& _rxViewController = nullptr,
                    const css::uno::Any& _Supplement = css::uno::Any()
                );

        /** notifies a document event, described by the given parameters, asynchronously

            Note that no event is actually notified before you called ->onDocumentInitialized.

            @precond
                the mutex is locked
        */
        void    notifyDocumentEventAsync(
                    const OUString& _EventName,
                    const css::uno::Reference< css::frame::XController2 >& _ViewController = nullptr,
                    const css::uno::Any& _Supplement = css::uno::Any()
                );

        /** notifies a document event to all registered listeners

            @precond
                the mutex is not locked
            @precond
                ->onDocumentInitialized has been called
        */
        void    notifyDocumentEvent(
                    const sal_Char* _pAsciiEventName,
                    const css::uno::Reference< css::frame::XController2 >& _rxViewController = nullptr,
                    const css::uno::Any& _rSupplement = css::uno::Any()
                )
        {
            notifyDocumentEvent( OUString::createFromAscii( _pAsciiEventName ), _rxViewController, _rSupplement );
        }

        /** notifies a document event to all registered listeners, asynchronously

            Note that no event is actually notified before you called ->onDocumentInitialized.

            @precond
                the mutex is locked
        */
        void    notifyDocumentEventAsync(
                    const sal_Char* _pAsciiEventName,
                    const css::uno::Reference< css::frame::XController2 >& _rxViewController = nullptr,
                    const css::uno::Any& _rSupplement = css::uno::Any()
                )
        {
            notifyDocumentEventAsync( OUString::createFromAscii( _pAsciiEventName ), _rxViewController, _rSupplement );
        }

    private:
        ::rtl::Reference< DocumentEventNotifier_Impl >   m_pImpl;
    };

} // namespace dbaccess

#endif // INCLUDED_DBACCESS_SOURCE_CORE_DATAACCESS_DOCUMENTEVENTNOTIFIER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
