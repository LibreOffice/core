/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: connectiondependent.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 15:18:38 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef DBACCESS_CONNECTION_DEPENDENT_HXX
#define DBACCESS_CONNECTION_DEPENDENT_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
/** === end UNO includes === **/

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif

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

        /** a hard reference to the connection we're working for

            This member is only valid as long as a EntryGuard is on the stack.
            The guard will, in its constructor, set the member, and reset it in its destructor.
            This ensures that the connection is only held hard when it's needed, and weak otherwise.
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
                                m_xConnection;

    protected:
        ::osl::Mutex&   getMutex() const { return m_aMutex; }

    protected:
        class EntryGuard;

    protected:
        ConnectionDependentComponent() { }

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

