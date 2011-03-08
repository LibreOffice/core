/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
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
