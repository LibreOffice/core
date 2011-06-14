/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*************************************************************************
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
************************************************************************/

#ifndef DBACCESS_DOCUMENTEVENTNOTIFIER_HXX
#define DBACCESS_DOCUMENTEVENTNOTIFIER_HXX

/** === begin UNO includes === **/
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/document/XDocumentEventListener.hpp>
/** === end UNO includes === **/

#include <rtl/ref.hxx>

namespace cppu
{
    class OWeakObject;
}

namespace dbaccess
{

    class DocumentEventNotifier_Impl;
    //====================================================================
    //= DocumentEventNotifier
    //====================================================================
    class DocumentEventNotifier
    {
    public:
        DocumentEventNotifier( ::cppu::OWeakObject& _rBroadcasterDocument, ::osl::Mutex& _rMutex );
        ~DocumentEventNotifier();

        void addLegacyEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::document::XEventListener >& _Listener );
        void removeLegacyEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::document::XEventListener >& _Listener );
        void addDocumentEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::document::XDocumentEventListener >& _Listener );
        void removeDocumentEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::document::XDocumentEventListener >& _Listener );

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
                    const ::rtl::OUString& _EventName,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController2 >& _rxViewController = NULL,
                    const ::com::sun::star::uno::Any& _Supplement = ::com::sun::star::uno::Any()
                );

        /** notifies a document event, described by the given parameters, asynchronously

            Note that no event is actually notified before you called ->onDocumentInitialized.

            @precond
                the mutex is locked
        */
        void    notifyDocumentEventAsync(
                    const ::rtl::OUString& _EventName,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController2 >& _ViewController = NULL,
                    const ::com::sun::star::uno::Any& _Supplement = ::com::sun::star::uno::Any()
                );

        /** notifies a document event to all registered listeners

            @precond
                the mutex is not locked
            @precond
                ->onDocumentInitialized has been called
        */
        void    notifyDocumentEvent(
                    const sal_Char* _pAsciiEventName,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController2 >& _rxViewController = NULL,
                    const ::com::sun::star::uno::Any& _rSupplement = ::com::sun::star::uno::Any()
                )
        {
            notifyDocumentEvent( ::rtl::OUString::createFromAscii( _pAsciiEventName ), _rxViewController, _rSupplement );
        }

        /** notifies a document event to all registered listeners, asynchronously

            Note that no event is actually notified before you called ->onDocumentInitialized.

            @precond
                the mutex is locked
        */
        void    notifyDocumentEventAsync(
                    const sal_Char* _pAsciiEventName,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController2 >& _rxViewController = NULL,
                    const ::com::sun::star::uno::Any& _rSupplement = ::com::sun::star::uno::Any()
                )
        {
            notifyDocumentEventAsync( ::rtl::OUString::createFromAscii( _pAsciiEventName ), _rxViewController, _rSupplement );
        }

    private:
        ::rtl::Reference< DocumentEventNotifier_Impl >   m_pImpl;
    };

} // namespace dbaccess

#endif // DBACCESS_DOCUMENTEVENTNOTIFIER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
