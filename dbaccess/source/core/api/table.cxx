/*************************************************************************
 *
 *  $RCSfile: table.cxx,v $
 *
 *  $Revision: 1.47 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-28 15:48:12 $
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
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
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
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
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
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _CONNECTIVITY_SDBCX_COLUMN_HXX_
#include <connectivity/sdbcx/VColumn.hxx>
#endif
#ifndef DBACORE_SDBCORETOOLS_HXX
#include "sdbcoretools.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETMETADATA_HPP_
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETMETADATASUPPLIER_HPP_
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#endif

using namespace dbaccess;
using namespace connectivity;
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
using namespace ::utl;

typedef ::std::map <sal_Int32, OTableColumn*, std::less <sal_Int32> > OColMap;

//==========================================================================
//= ODBTable
//==========================================================================
DBG_NAME(ODBTable)
//--------------------------------------------------------------------------
ODBTable::ODBTable(connectivity::sdbcx::OCollection* _pTables,const OConfigurationNode& _rTableConfig,
        const Reference< XConnection >& _rxConn,
        const ::rtl::OUString& _rCatalog,
        const ::rtl::OUString& _rSchema,
        const ::rtl::OUString& _rName,
        const ::rtl::OUString& _rType,
        const ::rtl::OUString& _rDesc) throw(SQLException)
    :OTable_Base(_pTables,_rxConn,_rxConn->getMetaData()->storesMixedCaseQuotedIdentifiers(), _rName, _rType, _rDesc, _rSchema, _rCatalog )
    ,OConfigurationFlushable(m_aMutex,_rTableConfig.isValid() ? _rTableConfig.cloneAsRoot() : OConfigurationTreeRoot())
    ,m_nPrivileges(0)
{
    DBG_CTOR(ODBTable, NULL);
    osl_incrementInterlockedCount( &m_refCount );
    try
    {
        DBG_ASSERT(getMetaData().is(), "ODBTable::ODBTable : invalid conn !");
        DBG_ASSERT(_rName.getLength(), "ODBTable::ODBTable : name !");
        // register our properties
        construct();
        refreshColumns();

        // load the settings from the configuration
        if(m_aConfigurationNode.isValid())
            // our own settings
            loadFrom(m_aConfigurationNode.openNode(CONFIGKEY_SETTINGS));

        // we don't collect the privileges here, this is potentially expensive. Instead we determine them on request.
        // (see getFastPropertyValue)
        m_nPrivileges = -1;
    }
    catch(Exception&)
    {
    }
    osl_decrementInterlockedCount( &m_refCount );

    // TODO : think about collecting the privileges here, as we can't ensure that in getFastPropertyValue, where
    // we do this at the moment, the statement needed can be supplied by the connection (for example the SQL-Server
    // ODBC driver does not allow more than one statement per connection, and in getFastPropertyValue it's more
    // likely that it's already used up than it's here.)
}
// -----------------------------------------------------------------------------
ODBTable::ODBTable(connectivity::sdbcx::OCollection* _pTables,  const Reference< XConnection >& _rxConn )
                throw(SQLException)
    :OTable_Base(_pTables,_rxConn, _rxConn->getMetaData()->storesMixedCaseQuotedIdentifiers())
    ,OConfigurationFlushable(m_aMutex)
    ,m_nPrivileges(-1)
{
    construct();
}
// -------------------------------------------------------------------------
ODBTable::~ODBTable()
{
    DBG_DTOR(ODBTable, NULL);
}
// -----------------------------------------------------------------------------
IMPLEMENT_FORWARD_REFCOUNT(ODBTable,OTable_Base)

//--------------------------------------------------------------------------
OColumn* ODBTable::createColumn(const ::rtl::OUString& _rName) const
{
    OColumn* pReturn = NULL;

    Reference< XNamed > xRet = NULL;
    if ( m_xDriverColumns.is() && m_xDriverColumns->hasByName(_rName) )
    {
        Reference<XPropertySet> xProp;
        m_xDriverColumns->getByName(_rName) >>= xProp;

        pReturn = new OTableColumnWrapper(xProp);
    }
    else
    {
        OColumns* pColumns = static_cast<OColumns*>(m_pColumns);
        Reference<XPropertySet> xProp(pColumns->createBaseObject(_rName),UNO_QUERY);
        pReturn = new OTableColumnWrapper( xProp );
    }
    return pReturn;
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
void SAL_CALL ODBTable::disposing()
{
    OTable_Base::disposing();
    OConfigurationFlushable::disposing();
}

//------------------------------------------------------------------------------
void ODBTable::getFastPropertyValue(Any& _rValue, sal_Int32 _nHandle) const
{
    if ((PROPERTY_ID_PRIVILEGES == _nHandle) && (-1 == m_nPrivileges))
    {   // somebody is asking for the privileges an we do not know them, yet
        const_cast<ODBTable*>(this)->m_nPrivileges = ::dbtools::getTablePrivileges(getMetaData(),m_CatalogName,m_SchemaName, m_Name);
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

    registerProperty(PROPERTY_PRIVILEGES, PROPERTY_ID_PRIVILEGES, PropertyAttribute::BOUND | PropertyAttribute::READONLY,
                    &m_nPrivileges, ::getCppuType(static_cast<sal_Int32*>(NULL)));

    registerMayBeVoidProperty(PROPERTY_TEXTLINECOLOR, PROPERTY_ID_TEXTLINECOLOR, PropertyAttribute::BOUND | PropertyAttribute::MAYBEVOID,
                    &m_aTextLineColor, ::getCppuType(static_cast<sal_Int32*>(NULL)));

    registerProperty(PROPERTY_TEXTEMPHASIS, PROPERTY_ID_TEXTEMPHASIS, PropertyAttribute::BOUND,
                    &m_nFontEmphasis, ::getCppuType(&m_nFontEmphasis));

    registerProperty(PROPERTY_TEXTRELIEF, PROPERTY_ID_TEXTRELIEF, PropertyAttribute::BOUND,
                    &m_nFontRelief, ::getCppuType(&m_nFontRelief));
}
// -----------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* ODBTable::createArrayHelper( sal_Int32 _nId) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    if(!_nId)
    {
        Property* pBegin    = aProps.getArray();
        Property* pEnd      = pBegin + aProps.getLength();
        for(;pBegin != pEnd;++pBegin)
        {
            if (0 == pBegin->Name.compareToAscii(PROPERTY_CATALOGNAME))
                pBegin->Attributes = PropertyAttribute::READONLY;
            else if (0 == pBegin->Name.compareToAscii(PROPERTY_SCHEMANAME))
                pBegin->Attributes = PropertyAttribute::READONLY;
            else if (0 == pBegin->Name.compareToAscii(PROPERTY_DESCRIPTION))
                pBegin->Attributes = PropertyAttribute::READONLY;
            else if (0 == pBegin->Name.compareToAscii(PROPERTY_NAME))
                pBegin->Attributes = PropertyAttribute::READONLY;
        }
    }

    return new ::cppu::OPropertyArrayHelper(aProps);
}
// -----------------------------------------------------------------------------
::cppu::IPropertyArrayHelper & SAL_CALL ODBTable::getInfoHelper()
{
    return *ODBTable_PROP::getArrayHelper(isNew() ? 1 : 0);
}
// -------------------------------------------------------------------------
// XServiceInfo
IMPLEMENT_SERVICE_INFO1(ODBTable, "com.sun.star.sdb.dbaccess.ODBTable", SERVICE_SDBCX_TABLE)
// -------------------------------------------------------------------------
Any SAL_CALL ODBTable::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet;
    if(rType == getCppuType( (Reference<XRename>*)0))
        return Any();
    if(rType == getCppuType( (Reference<XAlterTable>*)0))
        return Any();
    aRet = OTable_Base::queryInterface( rType);

    if(!aRet.hasValue())
        aRet = OConfigurationFlushable::queryInterface( rType);

    return aRet;
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL ODBTable::getTypes(  ) throw(RuntimeException)
{
    Type aRenameType = getCppuType( (Reference<XRename>*)0);
    Type aAlterType = getCppuType( (Reference<XAlterTable>*)0);

    Sequence< Type > aTypes(OTable_Base::getTypes());
    ::std::vector<Type> aOwnTypes;
    aOwnTypes.reserve(aTypes.getLength());

    const Type* pBegin = aTypes.getConstArray();
    const Type* pEnd = pBegin + aTypes.getLength();
    for(;pBegin != pEnd ;++pBegin)
    {
        if(*pBegin != aRenameType && *pBegin != aAlterType)
            aOwnTypes.push_back(*pBegin);
    }
    Sequence< Type > aRet(aOwnTypes.begin(),aOwnTypes.size());
    return ::comphelper::concatSequences(aRet,OConfigurationFlushable::getTypes());
}
// -----------------------------------------------------------------------------
void ODBTable::flush_NoBroadcast_NoCommit()
{
    if(m_aConfigurationNode.isValid())
    {
        storeTo(m_aConfigurationNode.openNode(CONFIGKEY_SETTINGS));

        OColumns* pColumns = static_cast<OColumns*>(m_pColumns);
        if ( pColumns )
        {
            Reference<XConnection> xCon = getConnection();
            pColumns->storeSettings( m_aConfigurationNode.openNode(CONFIGKEY_QRYDESCR_COLUMNS), getDataSourceNumberFormats( xCon ) );
        };
    }
}
// XRename,
//------------------------------------------------------------------------------
void SAL_CALL ODBTable::rename( const ::rtl::OUString& _rNewName ) throw(SQLException, ElementExistException, RuntimeException)
{
    throw SQLException(::rtl::OUString::createFromAscii("Driver does not support this function!"),*this,::rtl::OUString::createFromAscii("IM001"),0,Any());
}

// XAlterTable,
//------------------------------------------------------------------------------
void SAL_CALL ODBTable::alterColumnByName( const ::rtl::OUString& _rName, const Reference< XPropertySet >& _rxDescriptor ) throw(SQLException, NoSuchElementException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OTable_Linux::rBHelper.bDisposed);
    if(m_pColumns->hasByName(_rName))
    {
        ::rtl::OUString sSql = ::rtl::OUString::createFromAscii("ALTER TABLE ");
        ::rtl::OUString aQuote  = getMetaData()->getIdentifierQuoteString(  );
        ::rtl::OUString sComposedName;

        ::dbtools::composeTableName(getMetaData(),m_CatalogName,m_SchemaName,m_Name,sComposedName,sal_True,::dbtools::eInTableDefinitions);
        if(!sComposedName.getLength())
            ::dbtools::throwFunctionSequenceException(*this);

        sSql += sComposedName;
        sSql += ::rtl::OUString::createFromAscii(" ALTER ");
        sSql += ::dbtools::quoteName(aQuote,_rName);

        ::rtl::OUString sNewDefaultValue,sDefaultValue;

        Reference<XPropertySet> xColumn;
        m_pColumns->getByName(_rName) >>= xColumn;
        if(_rxDescriptor->getPropertySetInfo()->hasPropertyByName(PROPERTY_DEFAULTVALUE))
            _rxDescriptor->getPropertyValue(PROPERTY_DEFAULTVALUE) >>= sNewDefaultValue;
        if(xColumn.is() && xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_DEFAULTVALUE))
            xColumn->getPropertyValue(PROPERTY_DEFAULTVALUE) >>= sDefaultValue;

        if(sNewDefaultValue != sDefaultValue)
        {
            if(sNewDefaultValue.getLength())
            {
                sSql += ::rtl::OUString::createFromAscii(" SET DEFAULT ");
                sSql += sNewDefaultValue;
            }
            else
                sSql += ::rtl::OUString::createFromAscii(" DROP DEFAULT");
            OSL_ENSURE(getMetaData()->getConnection().is(),"Connection is null!");
            Reference< XStatement > xStmt = getMetaData()->getConnection()->createStatement(  );
            if(xStmt.is())
                xStmt->execute(sSql);
        }
        else
            // not supported
            throw SQLException(::rtl::OUString::createFromAscii("Driver does not support this function!"),*this,::rtl::OUString::createFromAscii("IM001"),0,Any());
    }
    else
        // not supported
        throw SQLException(::rtl::OUString::createFromAscii("Driver does not support this function!"),*this,::rtl::OUString::createFromAscii("IM001"),0,Any());
    m_pColumns->refresh();
}
// -------------------------------------------------------------------------
void ODBTable::refreshColumns()
{
    OTable_Base::refreshColumns();
    // our column's settings
    if ( m_aConfigurationNode.isValid() )
    {
        OColumns* pColumns = static_cast<OColumns*>(m_pColumns);
        Reference<XConnection> xCon = getConnection();
        pColumns->loadSettings( m_aConfigurationNode.openNode(CONFIGKEY_QRYDESCR_COLUMNS), getDataSourceNumberFormats( xCon ) );
    }

}
// -----------------------------------------------------------------------------
sal_Int64 SAL_CALL ODBTable::getSomething( const Sequence< sal_Int8 >& rId ) throw(RuntimeException)
{
    sal_Int64 nRet(0);
    if (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        nRet = (sal_Int64)this;
    else
    {
        nRet = OTable_Base::getSomething(rId);
        if(!nRet)
            nRet = OConfigurationFlushable::getSomething(rId);
    }
    return nRet;
}
// -----------------------------------------------------------------------------
Sequence< sal_Int8 > ODBTable::getUnoTunnelImplementationId()
{
    static ::cppu::OImplementationId * pId = 0;
    if (! pId)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! pId)
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}
// -----------------------------------------------------------------------------
Reference< XPropertySet > ODBTable::createEmptyObject()
{
    return new OTableColumnDescriptor();
}
// -----------------------------------------------------------------------------
sdbcx::OCollection* ODBTable::createColumns(const TStringVector& _rNames)
{
    OColumns* pCol = new OColumns(*this, m_aMutex, NULL, isCaseSensitive(), _rNames, this,this,
                                    getMetaData()->supportsAlterTableWithAddColumn(),
                                    getMetaData()->supportsAlterTableWithDropColumn());
    pCol->setParent(this);
    return pCol;
}
// -----------------------------------------------------------------------------
sdbcx::OCollection* ODBTable::createKeys(const TStringVector& _rNames)
{
    return new OKeys(this,m_aMutex,_rNames,NULL);
}
// -----------------------------------------------------------------------------
sdbcx::OCollection* ODBTable::createIndexes(const TStringVector& _rNames)
{
    return new OIndexes(this,m_aMutex,_rNames,NULL);
}
// -----------------------------------------------------------------------------




