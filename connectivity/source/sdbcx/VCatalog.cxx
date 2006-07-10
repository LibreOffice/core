/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VCatalog.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 14:39:15 $
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

#ifndef _CONNECTIVITY_SDBCX_CATALOG_HXX_
#include "connectivity/sdbcx/VCatalog.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_COLLECTION_HXX_
#include "connectivity/sdbcx/VCollection.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _CONNECTIVITY_SDBCX_COLLECTION_HXX_
#include "connectivity/sdbcx/VCollection.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_DESCRIPTOR_HXX_
#include "connectivity/sdbcx/VDescriptor.hxx"
#endif
#ifndef CONNECTIVITY_CONNECTION_HXX
#include "TConnection.hxx"
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include "connectivity/dbtools.hxx"
#endif

using namespace connectivity;
using namespace connectivity::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
//------------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO(OCatalog,"com.sun.star.comp.connectivity.OCatalog","com.sun.star.sdbcx.DatabaseDefinition")
//------------------------------------------------------------------------------
OCatalog::OCatalog(const Reference< XConnection> &_xConnection) : OCatalog_BASE(m_aMutex)
            ,connectivity::OSubComponent<OCatalog, OCatalog_BASE>(_xConnection, this)
            ,m_pTables(NULL)
            ,m_pViews(NULL)
            ,m_pGroups(NULL)
            ,m_pUsers(NULL)
{
    try
    {
        m_xMetaData = _xConnection->getMetaData();
    }
    catch(const Exception&)
    {
        OSL_ENSURE(0,"No Metadata available!");
    }
}
//-----------------------------------------------------------------------------
OCatalog::~OCatalog()
{
    delete m_pTables;
    delete m_pViews;
    delete m_pGroups;
    delete m_pUsers;
}
//-----------------------------------------------------------------------------
void SAL_CALL OCatalog::acquire() throw()
{
    OCatalog_BASE::acquire();
}
//------------------------------------------------------------------------------
void SAL_CALL OCatalog::release() throw()
{
    relase_ChildImpl();
}

//------------------------------------------------------------------------------
void SAL_CALL OCatalog::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    if(m_pTables)
        m_pTables->disposing();
    if(m_pViews)
        m_pViews->disposing();
    if(m_pGroups)
        m_pGroups->disposing();
    if(m_pUsers)
        m_pUsers->disposing();

    dispose_ChildImpl();
    OCatalog_BASE::disposing();
}
//------------------------------------------------------------------------------
// XTablesSupplier
Reference< XNameAccess > SAL_CALL OCatalog::getTables(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OCatalog_BASE::rBHelper.bDisposed);

    try
    {
        if(!m_pTables)
            refreshTables();
    }
    catch( const RuntimeException& )
    {
        // allowed to leave this method
        throw;
    }
    catch( const Exception& )
    {
        // allowed
    }

    return const_cast<OCatalog*>(this)->m_pTables;
}
// -------------------------------------------------------------------------
// XViewsSupplier
Reference< XNameAccess > SAL_CALL OCatalog::getViews(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OCatalog_BASE::rBHelper.bDisposed);

    try
    {
        if(!m_pViews)
            refreshViews();
    }
    catch( const RuntimeException& )
    {
        // allowed to leave this method
        throw;
    }
    catch( const Exception& )
    {
        // allowed
    }

    return const_cast<OCatalog*>(this)->m_pViews;
}
// -------------------------------------------------------------------------
// XUsersSupplier
Reference< XNameAccess > SAL_CALL OCatalog::getUsers(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OCatalog_BASE::rBHelper.bDisposed);

    try
    {
        if(!m_pUsers)
            refreshUsers();
    }
    catch( const RuntimeException& )
    {
        // allowed to leave this method
        throw;
    }
    catch( const Exception& )
    {
        // allowed
    }

    return const_cast<OCatalog*>(this)->m_pUsers;
}
// -------------------------------------------------------------------------
// XGroupsSupplier
Reference< XNameAccess > SAL_CALL OCatalog::getGroups(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OCatalog_BASE::rBHelper.bDisposed);

    try
    {
        if(!m_pGroups)
            refreshGroups();
    }
    catch( const RuntimeException& )
    {
        // allowed to leave this method
        throw;
    }
    catch( const Exception& )
    {
        // allowed
    }

    return const_cast<OCatalog*>(this)->m_pGroups;
}
// -----------------------------------------------------------------------------
::rtl::OUString OCatalog::buildName(const Reference< XRow >& _xRow)
{
    ::rtl::OUString sCatalog = _xRow->getString(1);
    if ( _xRow->wasNull() )
        sCatalog = ::rtl::OUString();
    ::rtl::OUString sSchema  = _xRow->getString(2);
    if ( _xRow->wasNull() )
        sSchema = ::rtl::OUString();
    ::rtl::OUString sTable   = _xRow->getString(3);
    if ( _xRow->wasNull() )
        sTable = ::rtl::OUString();

    ::rtl::OUString sComposedName(
        ::dbtools::composeTableName( m_xMetaData, sCatalog, sSchema, sTable, sal_False, ::dbtools::eInDataManipulation ) );
    return sComposedName;
}
// -----------------------------------------------------------------------------
void OCatalog::fillNames(Reference< XResultSet >& _xResult,TStringVector& _rNames)
{
    if ( _xResult.is() )
    {
        _rNames.reserve(20);
        Reference< XRow > xRow(_xResult,UNO_QUERY);
        while ( _xResult->next() )
        {
            _rNames.push_back( buildName(xRow) );
        }
        xRow = NULL;
        ::comphelper::disposeComponent(_xResult);
    }
}
// -------------------------------------------------------------------------
void ODescriptor::construct()
{
    sal_Int32 nAttrib = isNew() ? 0 : ::com::sun::star::beans::PropertyAttribute::READONLY;
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME), PROPERTY_ID_NAME ,nAttrib,&m_Name,::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
}
// -------------------------------------------------------------------------
ODescriptor::~ODescriptor()
{
}
// -----------------------------------------------------------------------------

