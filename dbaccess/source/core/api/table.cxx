/*************************************************************************
 *
 *  $RCSfile: table.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: oj $ $Date: 2000-10-17 10:18:12 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DBA_CORE_TABLE_HXX_
#include "table.hxx"
#endif
#ifndef _DBACORE_DEFINITIONCOLUMN_HXX_
#include <definitioncolumn.hxx>
#endif
#ifndef _DBA_SHARED_STUBS_HXX_
#include "stubs.hxx"
#endif
#ifndef _DBASHARED_STRINGCONSTANTS_HRC_
#include "stringconstants.hrc"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_ENUMHELPER_HXX_
#include <comphelper/enumhelper.hxx>
#endif
#ifndef _COMPHELPER_CONTAINER_HXX_
#include <comphelper/container.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif

#ifndef _COM_SUN_STAR_UTIL_XREFRESHLISTENER_HPP_
#include <com/sun/star/util/XRefreshListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_PRIVILEGE_HPP_
#include <com/sun/star/sdbcx/Privilege.hpp>
#endif
#ifndef DBACCESS_CORE_API_KEYS_HXX
#include "CKeys.hxx"
#endif
#ifndef DBACCESS_INDEXES_HXX_
#include "CIndexes.hxx"
#endif

using namespace dbaccess;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::osl;
using namespace ::comphelper;
using namespace ::cppu;

typedef ::std::map <sal_Int32, OTableColumn*, std::less <sal_Int32> > OColMap;

//==========================================================================
//= ODBTable
//==========================================================================
DBG_NAME(ODBTable)
//--------------------------------------------------------------------------
ODBTable::ODBTable(const Reference< XConnection >& _rxConn,
        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier >& _rxTable,
        const ::rtl::OUString& _rCatalog,
        const ::rtl::OUString& _rSchema,
        const ::rtl::OUString& _rName,
        const ::rtl::OUString& _rType,
        const ::rtl::OUString& _rDesc) throw(::com::sun::star::sdbc::SQLException)
        : connectivity::sdbcx::OTable(_rxConn->getMetaData()->storesMixedCaseQuotedIdentifiers(),_rName,_rType,_rDesc,_rSchema,_rCatalog)
        ,m_aConnection(_rxConn)
        ,m_nPrivileges(0)
        ,m_xTable(_rxTable)
{
    DBG_CTOR(ODBTable, NULL);
    osl_incrementInterlockedCount( &m_refCount );
    DBG_ASSERT(_rxConn.is(), "ODBTable::ODBTable : invalid conn !");
    DBG_ASSERT(_rName.getLength(), "ODBTable::ODBTable : name !");
    // register our properties
    construct();
    refreshColumns();
    refreshKeys();
    refreshIndexes();

    // we don't collect the privileges here, this is potentially expensive. Instead we determine them on request.
    // (see getFastPropertyValue)
    m_nPrivileges = -1;
    osl_decrementInterlockedCount( &m_refCount );

    // TODO : think about collecting the privileges here, as we can't ensure that in getFastPropertyValue, where
    // we do this at the moment, the statement needed can be supplied by the connection (for example the SQL-Server
    // ODBC driver does not allow more than one statement per connection, and in getFastPropertyValue it's more
    // likely that it's already used up than it's here.)
}
// -------------------------------------------------------------------------
ODBTable::~ODBTable()
{
    DBG_DTOR(ODBTable, NULL);
}

//--------------------------------------------------------------------------
Sequence< sal_Int8 > ODBTable::getImplementationId() throw (RuntimeException)
{
    static OImplementationId * pId = 0;
    if (! pId)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! pId)
        {
            static OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

// OComponentHelper
//------------------------------------------------------------------------------
void ODBTable::disposing()
{
    OTable_Base::disposing();
    MutexGuard aGuard(m_aMutex);
    //  m_aColumns.disposing();
    m_aConnection = Reference< XConnection > ();
    m_xTable = NULL;
}

//------------------------------------------------------------------------------
void ODBTable::getFastPropertyValue(Any& _rValue, sal_Int32 _nHandle) const
{
    if ((PROPERTY_ID_PRIVILEGES == _nHandle) && (-1 == m_nPrivileges))
    {   // somebody is asking for the privileges an we do not know them, yet
        const_cast<ODBTable*>(this)->m_nPrivileges = 0; // don't allow anything if something goes wrong
        try
        {
            Reference< XConnection > xConn(m_aConnection.get(), UNO_QUERY);
            Reference< XDatabaseMetaData > xMetaData = xConn.is() ? xConn->getMetaData() : Reference< XDatabaseMetaData >();
            Any aVal;
            if(m_CatalogName.getLength())
                aVal <<= m_CatalogName;
            Reference< XResultSet > xPrivileges = xMetaData.is() ? xMetaData->getTablePrivileges(aVal, m_SchemaName, m_Name) : Reference< XResultSet >();
            Reference< XRow > xCurrentRow(xPrivileges, UNO_QUERY);

            if (xCurrentRow.is())
            {
                ::rtl::OUString sUserWorkingFor = xMetaData->getUserName();
                xPrivileges->next();
                    // after creation the set is positioned before the first record, per definitionem

                ::rtl::OUString sPrivilege, sGrantee;
                while (!xPrivileges->isAfterLast())
                {
#ifdef DBG_UTIL
                    ::rtl::OUString sCat, sSchema, sName, sGrantor, sGrantable;
                    sCat = xCurrentRow->getString(1);
                    sSchema = xCurrentRow->getString(2);
                    sName = xCurrentRow->getString(3);
                    sGrantor = xCurrentRow->getString(4);
#endif
                    sGrantee = xCurrentRow->getString(5);
                    sPrivilege = xCurrentRow->getString(6);
#ifdef DBG_UTIL
                    sGrantable = xCurrentRow->getString(7);
#endif

                    xPrivileges->next();
                    if (sUserWorkingFor != sGrantee)
                        continue;

                    if (sPrivilege.compareToAscii("SELECT") == 0)
                        const_cast<ODBTable*>(this)->m_nPrivileges |= Privilege::SELECT;
                    else if (sPrivilege.compareToAscii("INSERT") == 0)
                        const_cast<ODBTable*>(this)->m_nPrivileges |= Privilege::INSERT;
                    else if (sPrivilege.compareToAscii("UPDATE") == 0)
                        const_cast<ODBTable*>(this)->m_nPrivileges |= Privilege::UPDATE;
                    else if (sPrivilege.compareToAscii("DELETE") == 0)
                        const_cast<ODBTable*>(this)->m_nPrivileges |= Privilege::DELETE;
                    else if (sPrivilege.compareToAscii("READ") == 0)
                        const_cast<ODBTable*>(this)->m_nPrivileges |= Privilege::READ;
                    else if (sPrivilege.compareToAscii("CREATE") == 0)
                        const_cast<ODBTable*>(this)->m_nPrivileges |= Privilege::CREATE;
                    else if (sPrivilege.compareToAscii("ALTER") == 0)
                        const_cast<ODBTable*>(this)->m_nPrivileges |= Privilege::ALTER;
                    else if (sPrivilege.compareToAscii("REFERENCE") == 0)
                        const_cast<ODBTable*>(this)->m_nPrivileges |= Privilege::REFERENCE;
                    else if (sPrivilege.compareToAscii("DROP") == 0)
                        const_cast<ODBTable*>(this)->m_nPrivileges |= Privilege::DROP;
                }
            }
            disposeComponent(xPrivileges);
        }
        catch(SQLException& e)
        {
            UNUSED(e);
            DBG_ERROR("ODBTable::ODBTable : could not collect the privileges !");
        }
    }

    OTable_Base::getFastPropertyValue(_rValue, _nHandle);
}
// -------------------------------------------------------------------------
void ODBTable::construct()
{
    OTable_Base::construct();

    registerProperty(PROPERTY_FILTER, PROPERTY_ID_FILTER, PropertyAttribute::BOUND,
                    &m_sFilter, ::getCppuType(&m_sFilter));

    registerProperty(PROPERTY_ORDER, PROPERTY_ID_ORDER, PropertyAttribute::BOUND,
                    &m_sOrder, ::getCppuType(&m_sOrder));

    registerProperty(PROPERTY_APPLYFILTER, PROPERTY_ID_APPLYFILTER, PropertyAttribute::BOUND,
                    &m_bApplyFilter, ::getBooleanCppuType());

    registerProperty(PROPERTY_FONT, PROPERTY_ID_FONT, PropertyAttribute::BOUND,
                    &m_aFont, ::getCppuType(&m_aFont));

    registerMayBeVoidProperty(PROPERTY_ROW_HEIGHT, PROPERTY_ID_ROW_HEIGHT, PropertyAttribute::BOUND | PropertyAttribute::MAYBEVOID,
                    &m_aRowHeight, ::getCppuType(static_cast<sal_Int32*>(NULL)));

    registerMayBeVoidProperty(PROPERTY_TEXTCOLOR, PROPERTY_ID_TEXTCOLOR, PropertyAttribute::BOUND | PropertyAttribute::MAYBEVOID,
                    &m_aTextColor, ::getCppuType(static_cast<sal_Int32*>(NULL)));
}
// -------------------------------------------------------------------------
// XServiceInfo
IMPLEMENT_SERVICE_INFO1(ODBTable, "com.sun.star.sdb.dbaccess.ODBTable", SERVICE_SDBCX_TABLE)
// -------------------------------------------------------------------------
Any SAL_CALL ODBTable::queryInterface( const Type & rType ) throw(RuntimeException)
{
    if(rType == getCppuType( (Reference<XRename>*)0))
        return Any();
    if(rType == getCppuType( (Reference<XAlterTable>*)0))
        return Any();

    return OTable_Base::queryInterface( rType);
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL ODBTable::getTypes(  ) throw(RuntimeException)
{
    Type aRenameType = getCppuType( (Reference<XRename>*)0);
    Type aAlterType = getCppuType( (Reference<XAlterTable>*)0);

    Sequence< Type > aTypes(OTable_Base::getTypes());
    Sequence< Type > aRet(aTypes.getLength()-2);

    const Type* pBegin = aTypes.getConstArray();
    const Type* pEnd = pBegin + aTypes.getLength();
    for(sal_Int32 i=0;pBegin != pEnd ;++pBegin)
    {
        if(*pBegin != aRenameType && *pBegin != aAlterType)
        {
            aRet.getArray()[i++] = *pBegin;
        }
    }
    return aRet;
}
// XDataDescriptorFactory
//------------------------------------------------------------------------------
// XRename,
//------------------------------------------------------------------------------
void SAL_CALL ODBTable::rename( const ::rtl::OUString& _rNewName ) throw(SQLException, ElementExistException, RuntimeException)
{
    // not supported
    throw SQLException();
}

// XAlterTable,
//------------------------------------------------------------------------------
void SAL_CALL ODBTable::alterColumnByName( const ::rtl::OUString& _rName, const Reference< XPropertySet >& _rxDescriptor ) throw(SQLException, NoSuchElementException, RuntimeException)
{
    // not supported
    throw SQLException();
}

//------------------------------------------------------------------------------
void SAL_CALL ODBTable::alterColumnByIndex( sal_Int32 _nIndex, const Reference< XPropertySet >& _rxDescriptor ) throw(SQLException, IndexOutOfBoundsException, RuntimeException)
{
    // not supported
    throw SQLException();
}
// -------------------------------------------------------------------------
void ODBTable::refreshColumns()
{
    ::std::vector< ::rtl::OUString> aVector;
    Reference<XNameAccess> xNames;
    if(m_xTable.is())
    {
        xNames = m_xTable->getColumns();
        Sequence< ::rtl::OUString> aNames = xNames->getElementNames();
        const ::rtl::OUString* pBegin   = aNames.getConstArray();
        const ::rtl::OUString* pEnd     = pBegin + aNames.getLength();
        for(;pBegin != pEnd;++pBegin)
            aVector.push_back(*pBegin);
    }
    else
    {
        Any aVal;
        if(m_CatalogName.getLength())
            aVal <<= m_CatalogName;
        Reference< XResultSet > xResult = Reference<XConnection>(m_aConnection)->getMetaData()->getColumns(aVal,
                                        m_SchemaName,m_Name,::rtl::OUString::createFromAscii("%"));

        if(xResult.is())
        {
            Reference< XRow > xRow(xResult,UNO_QUERY);
            while(xResult->next())
                aVector.push_back(xRow->getString(4));
        }


    }

    OColumns* pCol = new OColumns(*this,m_aMutex,xNames,isCaseSensitive(),aVector,
                                getConnection()->getMetaData()->supportsAlterTableWithAddColumn(),
                                getConnection()->getMetaData()->supportsAlterTableWithDropColumn());
    pCol->setParent(this);

    if(m_pColumns)
        delete m_pColumns;

    m_pColumns  = pCol;
}
// -------------------------------------------------------------------------
void ODBTable::refreshPrimaryKeys(std::vector< ::rtl::OUString>& _rKeys)
{
    Any aVal;
    if(m_CatalogName.getLength())
        aVal <<= m_CatalogName;

    Reference< XResultSet > xResult = Reference<XConnection>(m_aConnection)->getMetaData()->getPrimaryKeys(aVal,m_SchemaName,m_Name);

    if(xResult.is())
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        if(xResult->next()) // there can be only one primary key
        {
            ::rtl::OUString aPkName = xRow->getString(6);
            _rKeys.push_back(aPkName);
        }
    }
}
// -------------------------------------------------------------------------
void ODBTable::refreshForgeinKeys(std::vector< ::rtl::OUString>& _rKeys)
{
    Any aVal;
    if(m_CatalogName.getLength())
        aVal <<= m_CatalogName;

    Reference< XResultSet > xResult = Reference<XConnection>(m_aConnection)->getMetaData()->getExportedKeys(aVal,m_SchemaName,m_Name);

    if(xResult.is())
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        while(xResult->next())
            _rKeys.push_back(xRow->getString(12));
    }
}
// -------------------------------------------------------------------------
void ODBTable::refreshKeys()
{
    ::std::vector< ::rtl::OUString> aVector;

    refreshPrimaryKeys(aVector);
    refreshForgeinKeys(aVector);
    if(m_pKeys)
        delete m_pKeys;
    m_pKeys     = new OKeys(this,m_aMutex,aVector);
}
// -------------------------------------------------------------------------
void ODBTable::refreshIndexes()
{
    /*
    Any aVal;
    if(m_CatalogName.getLength())
        aVal <<= m_CatalogName;

    ::std::vector< ::rtl::OUString> aVector;
    // fill indexes
    Reference< XResultSet > xResult = Reference<XConnection>(m_aConnection)->getMetaData()->getIndexInfo(aVal,
    m_SchemaName,m_Name,sal_False,sal_False);

    if(xResult.is())
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        ::rtl::OUString aName,aDot = ::rtl::OUString::createFromAscii(".");
        while(xResult->next())
        {
            aName = xRow->getString(5);
            if(aName.getLength())
                aName += aDot;
            aName += xRow->getString(6);
            if(aName.getLength())
                aVector.push_back(aName);
        }
    }

    if(m_pIndexes)
        delete m_pIndexes;
    m_pIndexes  = new OIndexes(this,m_aMutex,aVector);
    */
}


