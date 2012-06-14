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
#ifndef DBA_CONTAINERLISTENER_HXX
#define DBA_CONTAINERLISTENER_HXX

#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/container/XContainerApproveListener.hpp>

#include <cppuhelper/weak.hxx>

namespace dbaccess
{

    //==========================================================================
    //= OContainerListener
    //==========================================================================
    typedef ::cppu::WeakImplHelper2 <   ::com::sun::star::container::XContainerListener
                                    ,   ::com::sun::star::container::XContainerApproveListener
                                    >   OContainerListener_BASE;

    /** is helper class to avoid a cycle in refcount
    */
    class OContainerListener : public OContainerListener_BASE
    {
        ::osl::Mutex&       m_rMutex;
        OWeakObject&        m_rDestination;
        bool                m_bDisposed;

    public:
        OContainerListener( OWeakObject& _rDestination, ::osl::Mutex& _rMutex )
            :m_rMutex( _rMutex )
            ,m_rDestination( _rDestination )
            ,m_bDisposed( false )
        {
        }

        // XContainerApproveListener
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XVeto > SAL_CALL approveInsertElement( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XVeto > SAL_CALL approveReplaceElement( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XVeto > SAL_CALL approveRemoveElement( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        // XContainerListener
        virtual void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

        void SAL_CALL dispose()
        {
            m_bDisposed = true;
        }

    protected:
        virtual ~OContainerListener();
    };

}   // namespace dbaccess

#endif // DBA_CONTAINERLISTENER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
