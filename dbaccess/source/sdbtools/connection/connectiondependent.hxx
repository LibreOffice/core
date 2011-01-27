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

#ifndef DBACCESS_CONNECTION_DEPENDENT_HXX
#define DBACCESS_CONNECTION_DEPENDENT_HXX

/** === begin UNO includes === **/
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
/** === end UNO includes === **/

#include <comphelper/componentcontext.hxx>
#include <cppuhelper/weakref.hxx>
#include <osl/mutex.hxx>

//........................................................................
namespace sdbtools
{
//........................................................................

    //====================================================================
    //= ConnectionDependentComponent
    //====================================================================
    class ConnectionDependentComponent
    {
    private:
        mutable ::osl::Mutex    m_aMutex;
        ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XConnection >
                                m_aConnection;
        ::comphelper::ComponentContext
                                m_aContext;

        /** a hard reference to the connection we're working for

            This member is only valid as long as a EntryGuard is on the stack.
            The guard will, in its constructor, set the member, and reset it in its destructor.
            This ensures that the connection is only held hard when it's needed, and weak otherwise.
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
                                m_xConnection;

    protected:
        ::osl::Mutex&   getMutex() const { return m_aMutex; }

        const ::comphelper::ComponentContext&
                        getContext() const { return m_aContext; }

    protected:
        class EntryGuard;

    protected:
        ConnectionDependentComponent( const ::comphelper::ComponentContext& _rContext )
            :m_aContext( _rContext )
        {
        }

        /** sets the connection we depend on.

            To be called exactly once.

            @param  _rxConnection
                the connection to set
        */
        void    setWeakConnection( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection )
        {
            m_aConnection = _rxConnection;
        }

        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >&
                getConnection() const { return m_xConnection; }

    public:
    struct GuardAccess;
    friend struct GuardAccess;
        /** helper for granting exclusive access to various other methods
        */
        struct GuardAccess { friend class EntryGuard; private: GuardAccess() { } };

        ::osl::Mutex&   getMutex( GuardAccess ) const { return m_aMutex; }

        inline bool acquireConnection( GuardAccess )
        {
            m_xConnection = (::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >)m_aConnection;
            return m_xConnection.is();
        }
        inline void releaseConnection( GuardAccess )
        {
            m_xConnection.clear();
        }
    };

    //====================================================================
    //= ConnectionDependentComponent::EntryGuard
    //====================================================================
    /** a class for guarding methods of a connection-dependent component

        This class serves multiple purposes:
        <ul><li>It ensures multi-threading safety by guarding the component's mutex
                as long as it lives.</li>
            <li>It ensures that the component's connection is alive. The constructor
                throws a DisposedException if no hard reference to the connection can
                be obtained.</li>
        </ul>
    */
    class ConnectionDependentComponent::EntryGuard
    {
    private:
        ::osl::MutexGuard               m_aMutexGuard;
        ConnectionDependentComponent&   m_rComponent;

    public:
        EntryGuard( ConnectionDependentComponent& _rComponent )
            :m_aMutexGuard( _rComponent.getMutex( ConnectionDependentComponent::GuardAccess() ) )
            ,m_rComponent( _rComponent )
        {
            if ( !m_rComponent.acquireConnection( ConnectionDependentComponent::GuardAccess() ) )
                throw ::com::sun::star::lang::DisposedException();
        }

        ~EntryGuard()
        {
            m_rComponent.releaseConnection( ConnectionDependentComponent::GuardAccess() );
        }
    };


//........................................................................
} // namespace sdbtools
//........................................................................

#endif // DBACCESS_CONNECTION_DEPENDENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
