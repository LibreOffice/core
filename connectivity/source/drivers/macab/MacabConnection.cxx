/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MacabConnection.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 07:56:15 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"

#include "MacabConnection.hxx"
#ifndef _CONNECTIVITY_MACAB_ADDRESSBOOK_HXX_
#include "MacabAddressBook.hxx"
#endif
#ifndef _CONNECTIVITY_MACAB_DATABASEMETADATA_HXX_
#include "MacabDatabaseMetaData.hxx"
#endif
#ifndef _CONNECTIVITY_MACAB_STATEMENT_HXX_
#include "MacabStatement.hxx"
#endif
#ifndef _CONNECTIVITY_MACAB_PREPAREDSTATEMENT_HXX_
#include "MacabPreparedStatement.hxx"
#endif
#ifndef _CONNECTIVITY_MACAB_DRIVER_HXX_
#include "MacabDriver.hxx"
#endif
#ifndef _CONNECTIVITY_MACAB_CATALOG_HXX_
#include "MacabCatalog.hxx"
#endif

#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_TRANSACTIONISOLATION_HPP_
#include <com/sun/star/sdbc/TransactionIsolation.hpp>
#endif

using namespace connectivity::macab;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;

IMPLEMENT_SERVICE_INFO(MacabConnection, "com.sun.star.sdbc.drivers.MacabConnection", "com.sun.star.sdbc.Connection")
//-----------------------------------------------------------------------------
MacabConnection::MacabConnection(MacabDriver*   _pDriver)
         : OSubComponent<MacabConnection, MacabConnection_BASE>((::cppu::OWeakObject*)_pDriver, this),
         m_pAddressBook(NULL),
         m_pDriver(_pDriver)
{
    m_pDriver->acquire();
}
//-----------------------------------------------------------------------------
MacabConnection::~MacabConnection()
{
    if (!isClosed())
        close();

    m_pDriver->release();
    m_pDriver = NULL;
}
//-----------------------------------------------------------------------------
void SAL_CALL MacabConnection::release() throw()
{
    relase_ChildImpl();
}
// -----------------------------------------------------------------------------
void MacabConnection::construct(const ::rtl::OUString&, const Sequence< PropertyValue >&) throw(SQLException)
{
    osl_incrementInterlockedCount( &m_refCount );

    // get the Mac OS X shared address book
    m_pAddressBook = new MacabAddressBook();

    osl_decrementInterlockedCount( &m_refCount );
}
// XServiceInfo
// --------------------------------------------------------------------------------
Reference< XStatement > SAL_CALL MacabConnection::createStatement(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);

    // create a statement
    // the statement can only be executed once
    Reference< XStatement > xReturn = new MacabStatement(this);
    m_aStatements.push_back(WeakReferenceHelper(xReturn));
    return xReturn;
}
// --------------------------------------------------------------------------------
Reference< XPreparedStatement > SAL_CALL MacabConnection::prepareStatement( const ::rtl::OUString& _sSql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);

    // create a statement
    // the statement can only be executed more than once
    Reference< XPreparedStatement > xReturn = new MacabPreparedStatement(this, _sSql);
    m_aStatements.push_back(WeakReferenceHelper(xReturn));
    return xReturn;
}
// --------------------------------------------------------------------------------
Reference< XPreparedStatement > SAL_CALL MacabConnection::prepareCall( const ::rtl::OUString& ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);

    // not implemented yet :-) a task to do
    return NULL;
}
// --------------------------------------------------------------------------------
::rtl::OUString SAL_CALL MacabConnection::nativeSQL( const ::rtl::OUString& _sSql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    // when you need to transform SQL92 to you driver specific you can do it here

    return _sSql;
}
// --------------------------------------------------------------------------------
void SAL_CALL MacabConnection::setAutoCommit( sal_Bool ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);
    // here you  have to set your commit mode please have a look at the jdbc documentation to get a clear explanation
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL MacabConnection::getAutoCommit(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);
    // you have to distinguish which if you are in autocommit mode or not
    // at normal case true should be fine here

    return sal_True;
}
// --------------------------------------------------------------------------------
void SAL_CALL MacabConnection::commit(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);

    // when you database does support transactions you should commit here
}
// --------------------------------------------------------------------------------
void SAL_CALL MacabConnection::rollback(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);

    // same as commit but for the other case
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL MacabConnection::isClosed(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    // just simple -> we are closed when we are disposed, that means someone called dispose(); (XComponent)
    return MacabConnection_BASE::rBHelper.bDisposed;
}
// --------------------------------------------------------------------------------
Reference< XDatabaseMetaData > SAL_CALL MacabConnection::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);

    // here we have to create the class with biggest interface
    // The answer is 42 :-)
    Reference< XDatabaseMetaData > xMetaData = m_xMetaData;
    if (!xMetaData.is())
    {
        xMetaData = new MacabDatabaseMetaData(this); // need the connection because it can return it
        m_xMetaData = xMetaData;
    }

    return xMetaData;
}
// --------------------------------------------------------------------------------
void SAL_CALL MacabConnection::setReadOnly( sal_Bool ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);

    // set you connection to readonly
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL MacabConnection::isReadOnly(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);

    // return if your connection to readonly
    return sal_False;
}
// --------------------------------------------------------------------------------
void SAL_CALL MacabConnection::setCatalog( const ::rtl::OUString& ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);

    // if your database doesn't work with catalogs you go to next method otherwise you kjnow what to do
}
// --------------------------------------------------------------------------------
::rtl::OUString SAL_CALL MacabConnection::getCatalog(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);


    // return your current catalog
    return ::rtl::OUString();
}
// --------------------------------------------------------------------------------
void SAL_CALL MacabConnection::setTransactionIsolation( sal_Int32 ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);

    // set your isolation level
    // please have a look at @see com.sun.star.sdbc.TransactionIsolation
}
// --------------------------------------------------------------------------------
sal_Int32 SAL_CALL MacabConnection::getTransactionIsolation(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);


    // please have a look at @see com.sun.star.sdbc.TransactionIsolation
    return TransactionIsolation::NONE;
}
// --------------------------------------------------------------------------------
Reference< ::com::sun::star::container::XNameAccess > SAL_CALL MacabConnection::getTypeMap(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);

    // if your driver has special database types you can return it here

    return NULL;
}
// --------------------------------------------------------------------------------
void SAL_CALL MacabConnection::setTypeMap( const Reference< ::com::sun::star::container::XNameAccess >& ) throw(SQLException, RuntimeException)
{
    // the other way around
}
// --------------------------------------------------------------------------------
// XCloseable
void SAL_CALL MacabConnection::close(  ) throw(SQLException, RuntimeException)
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(MacabConnection_BASE::rBHelper.bDisposed);
    }
    dispose();
}
// --------------------------------------------------------------------------------
// XWarningsSupplier
Any SAL_CALL MacabConnection::getWarnings(  ) throw(SQLException, RuntimeException)
{
    // when you collected some warnings -> return it
    return Any();
}
// --------------------------------------------------------------------------------
void SAL_CALL MacabConnection::clearWarnings(  ) throw(SQLException, RuntimeException)
{
    // you should clear your collected warnings here
}
//------------------------------------------------------------------------------
void MacabConnection::disposing()
{
    // we noticed that we should be destroied in near future so we have to dispose our statements
    ::osl::MutexGuard aGuard(m_aMutex);

    for (OWeakRefArray::iterator i = m_aStatements.begin(); m_aStatements.end() != i; ++i)
    {
        Reference< XComponent > xComp(i->get(), UNO_QUERY);
        if (xComp.is())
            xComp->dispose();
    }
    m_aStatements.clear();

    if (m_pAddressBook != NULL)
    {
        delete m_pAddressBook;
        m_pAddressBook = NULL;
    }

    m_xMetaData = ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XDatabaseMetaData>();

    dispose_ChildImpl();
    MacabConnection_BASE::disposing();
}
// -----------------------------------------------------------------------------
Reference< XTablesSupplier > SAL_CALL MacabConnection::createCatalog()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    Reference< XTablesSupplier > xTab = m_xCatalog;
    if (!m_xCatalog.is())
    {
        MacabCatalog *pCat = new MacabCatalog(this);
        xTab = pCat;
        m_xCatalog = xTab;
    }
    return xTab;
}
// -----------------------------------------------------------------------------
MacabAddressBook* MacabConnection::getAddressBook() const
{
    return m_pAddressBook;
}
// -----------------------------------------------------------------------------
extern "C" void*  SAL_CALL createMacabConnection( void* _pDriver )
{
    MacabConnection* pConnection = new MacabConnection( static_cast< MacabDriver* >( _pDriver ) );
    // by definition, the pointer crossing library boundaries as void ptr is acquired once
    pConnection->acquire();
    return pConnection;
}
