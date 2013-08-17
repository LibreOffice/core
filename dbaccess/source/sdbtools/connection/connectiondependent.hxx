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

#ifndef DBACCESS_CONNECTION_DEPENDENT_HXX
#define DBACCESS_CONNECTION_DEPENDENT_HXX

#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/weakref.hxx>
#include <osl/mutex.hxx>

namespace sdbtools
{

    // ConnectionDependentComponent
    class ConnectionDependentComponent
    {
    private:
        mutable ::osl::Mutex    m_aMutex;
        ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XConnection >
                                m_aConnection;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
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

        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >&
                        getContext() const { return m_aContext; }

    protected:
        class EntryGuard;

    protected:
        ConnectionDependentComponent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > & _rContext )
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

    // ConnectionDependentComponent::EntryGuard
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

} // namespace sdbtools

#endif // DBACCESS_CONNECTION_DEPENDENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
