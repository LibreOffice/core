/*************************************************************************
 *
 *  $RCSfile: TableDeco.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: fs $ $Date: 2001-08-30 08:03:17 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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

#ifndef _DBA_CORE_TABLEDECORATOR_HXX_
#include "TableDeco.hxx"
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
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
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
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
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
using namespace ::utl;

//==========================================================================
//= ODBTableDecorator
//==========================================================================
DBG_NAME(ODBTableDecorator)
//--------------------------------------------------------------------------
ODBTableDecorator::ODBTableDecorator(
        const OConfigurationNode& _rTableConfig, const Reference< XDatabaseMetaData >& _rxMetaData,
        const Reference< XColumnsSupplier >& _rxTable, const Reference< XNumberFormatsSupplier >& _rxNumberFormats ) throw(SQLException)
    :OTableDescriptor_BASE(m_aMutex)
    ,ODataSettings(OTableDescriptor_BASE::rBHelper)
    ,OConfigurationFlushable(m_aMutex,_rTableConfig.cloneAsRoot())
    ,m_nPrivileges(0)
    ,m_xTable(_rxTable)
    ,m_xMetaData(_rxMetaData)
    ,m_xNumberFormats( _rxNumberFormats )
    ,m_pColumns(NULL)
{
    DBG_CTOR(ODBTableDecorator, NULL);
    osl_incrementInterlockedCount( &m_refCount );

    DBG_ASSERT(_rxMetaData.is(), "ODBTableDecorator::ODBTableDecorator : invalid conn !");
    // register our properties
    construct();

    // load the settings from the configuration
    if(m_aConfigurationNode.isValid())
        loadFrom(m_aConfigurationNode.openNode(CONFIGKEY_SETTINGS));

    // we don't collect the privileges here, this is potentially expensive. Instead we determine them on request.
    // (see getFastPropertyValue)
    m_nPrivileges = -1;
    osl_decrementInterlockedCount( &m_refCount );

    // TODO : think about collecting the privileges here, as we can't ensure that in getFastPropertyValue, where
    // we do this at the moment, the statement needed can be supplied by the connection (for example the SQL-Server
    // ODBC driver does not allow more than one statement per connection, and in getFastPropertyValue it's more
    // likely that it's already used up than it's here.)
}
// -----------------------------------------------------------------------------
ODBTableDecorator::ODBTableDecorator(   const Reference< XDatabaseMetaData >& _rxMetaData,
        const Reference< XColumnsSupplier >& _rxNewTable, const Reference< XNumberFormatsSupplier >& _rxNumberFormats ) throw(SQLException)
    :OTableDescriptor_BASE(m_aMutex)
    ,ODataSettings(OTableDescriptor_BASE::rBHelper)
    ,OConfigurationFlushable(m_aMutex)
    ,m_nPrivileges(-1)
    ,m_xMetaData(_rxMetaData)
    ,m_xTable(_rxNewTable)
    ,m_xNumberFormats( _rxNumberFormats )
    ,m_pColumns(NULL)
{
    DBG_CTOR(ODBTableDecorator, NULL);
    construct();
}
// -------------------------------------------------------------------------
ODBTableDecorator::~ODBTableDecorator()
{
    DBG_DTOR(ODBTableDecorator, NULL);
    if(m_pColumns)
        delete m_pColumns;
}

//--------------------------------------------------------------------------
Sequence< sal_Int8 > ODBTableDecorator::getImplementationId() throw (RuntimeException)
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
void SAL_CALL ODBTableDecorator::disposing()
{
    OTableDescriptor_BASE::disposing();
    OConfigurationFlushable::disposing();

    MutexGuard aGuard(m_aMutex);
    m_xTable        = NULL;
    m_xMetaData     = NULL;
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL ODBTableDecorator::convertFastPropertyValue(
                            Any & rConvertedValue,
                            Any & rOldValue,
                            sal_Int32 nHandle,
                            const Any& rValue )
                                throw (::com::sun::star::lang::IllegalArgumentException)
{
    sal_Bool bRet = sal_True;
    switch(nHandle)
    {
        case PROPERTY_ID_PRIVILEGES:
        case PROPERTY_ID_FILTER:
        case PROPERTY_ID_ORDER:
        case PROPERTY_ID_APPLYFILTER:
        case PROPERTY_ID_FONT:
        case PROPERTY_ID_ROW_HEIGHT:
        case PROPERTY_ID_TEXTCOLOR:
        case PROPERTY_ID_TEXTLINECOLOR:
        case PROPERTY_ID_TEXTEMPHASIS:
        case PROPERTY_ID_TEXTRELIEF:
            bRet = ODataSettings::convertFastPropertyValue(rConvertedValue, rOldValue,nHandle,rValue);
            break;

        default:
            {
                Any aValue;
                getFastPropertyValue(aValue,nHandle);
                bRet = ::comphelper::tryPropertyValue(rConvertedValue,rOldValue,rValue,aValue,::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
            }
            break; // we assume that it works
    }
    return bRet;
}
// -----------------------------------------------------------------------------
void ODBTableDecorator::setFastPropertyValue_NoBroadcast(sal_Int32 _nHandle, const Any& _rValue) throw (Exception)
{
    switch(_nHandle)
    {
        case PROPERTY_ID_PRIVILEGES:
            OSL_ENSURE(0,"Property is readonly!");
        case PROPERTY_ID_FILTER:
        case PROPERTY_ID_ORDER:
        case PROPERTY_ID_APPLYFILTER:
        case PROPERTY_ID_FONT:
        case PROPERTY_ID_ROW_HEIGHT:
        case PROPERTY_ID_TEXTCOLOR:
        case PROPERTY_ID_TEXTLINECOLOR:
        case PROPERTY_ID_TEXTEMPHASIS:
        case PROPERTY_ID_TEXTRELIEF:
            ODataSettings::setFastPropertyValue_NoBroadcast(_nHandle, _rValue);
            break;
        case PROPERTY_ID_CATALOGNAME:
            {
                Reference<XPropertySet> xProp(m_xTable,UNO_QUERY);
                xProp->setPropertyValue(PROPERTY_CATALOGNAME,_rValue);
            }
            break;
        case PROPERTY_ID_SCHEMANAME:
            {
                Reference<XPropertySet> xProp(m_xTable,UNO_QUERY);
                xProp->setPropertyValue(PROPERTY_SCHEMANAME,_rValue);
            }
            break;
        case PROPERTY_ID_NAME:
            {
                Reference<XPropertySet> xProp(m_xTable,UNO_QUERY);
                xProp->setPropertyValue(PROPERTY_NAME,_rValue);
            }
            break;
        case PROPERTY_ID_DESCRIPTION:
            {
                Reference<XPropertySet> xProp(m_xTable,UNO_QUERY);
                xProp->setPropertyValue(PROPERTY_DESCRIPTION,_rValue);
            }
            break;
        case PROPERTY_ID_TYPE:
            {
                Reference<XPropertySet> xProp(m_xTable,UNO_QUERY);
                xProp->setPropertyValue(PROPERTY_TYPE,_rValue);
            }
            break;
    }
}
//------------------------------------------------------------------------------
void ODBTableDecorator::getFastPropertyValue(Any& _rValue, sal_Int32 _nHandle) const
{
    switch(_nHandle)
    {
        case PROPERTY_ID_PRIVILEGES:
            if(-1 == m_nPrivileges)
                fillPrivileges();

        case PROPERTY_ID_FILTER:
        case PROPERTY_ID_ORDER:
        case PROPERTY_ID_APPLYFILTER:
        case PROPERTY_ID_FONT:
        case PROPERTY_ID_ROW_HEIGHT:
        case PROPERTY_ID_TEXTCOLOR:
        case PROPERTY_ID_TEXTLINECOLOR:
        case PROPERTY_ID_TEXTEMPHASIS:
        case PROPERTY_ID_TEXTRELIEF:
            ODataSettings::getFastPropertyValue(_rValue, _nHandle);
            break;
        case PROPERTY_ID_CATALOGNAME:
            {
                Reference<XPropertySet> xProp(m_xTable,UNO_QUERY);
                _rValue = xProp->getPropertyValue(PROPERTY_CATALOGNAME);
            }
            break;
        case PROPERTY_ID_SCHEMANAME:
            {
                Reference<XPropertySet> xProp(m_xTable,UNO_QUERY);
                _rValue = xProp->getPropertyValue(PROPERTY_SCHEMANAME);
            }
            break;
        case PROPERTY_ID_NAME:
            {
                Reference<XPropertySet> xProp(m_xTable,UNO_QUERY);
                _rValue = xProp->getPropertyValue(PROPERTY_NAME);
            }
            break;
        case PROPERTY_ID_DESCRIPTION:
            {
                Reference<XPropertySet> xProp(m_xTable,UNO_QUERY);
                _rValue = xProp->getPropertyValue(PROPERTY_DESCRIPTION);
            }
            break;
        case PROPERTY_ID_TYPE:
            {
                Reference<XPropertySet> xProp(m_xTable,UNO_QUERY);
                _rValue = xProp->getPropertyValue(PROPERTY_TYPE);
            }
            break;
        default:
            OSL_ENSURE(0,"Invalid Handle for table");
    }
}
// -------------------------------------------------------------------------
void ODBTableDecorator::construct()
{
    registerProperty(PROPERTY_PRIVILEGES, PROPERTY_ID_PRIVILEGES, PropertyAttribute::BOUND  | PropertyAttribute::READONLY,
                    &m_nPrivileges, ::getCppuType(static_cast<sal_Int32*>(NULL)));
}
// -----------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* ODBTableDecorator::createArrayHelper(sal_Int32 _nId) const
{
    Reference<XPropertySet> xProp(m_xTable,UNO_QUERY);
    Reference<XPropertySetInfo> xInfo = xProp->getPropertySetInfo();

    Sequence< Property > aTableProps = xInfo->getProperties();
    Property* pBegin = aTableProps.getArray();
    Property* pEnd = pBegin + aTableProps.getLength();
    for (;pBegin != pEnd ; ++pBegin)
    {
        if (0 == pBegin->Name.compareToAscii(PROPERTY_CATALOGNAME))
            pBegin->Handle = PROPERTY_ID_CATALOGNAME;
        else if (0 ==pBegin->Name.compareToAscii(PROPERTY_SCHEMANAME))
            pBegin->Handle = PROPERTY_ID_SCHEMANAME;
        else if (0 ==pBegin->Name.compareToAscii(PROPERTY_NAME))
            pBegin->Handle = PROPERTY_ID_NAME;
        else if (0 ==pBegin->Name.compareToAscii(PROPERTY_DESCRIPTION))
            pBegin->Handle = PROPERTY_ID_DESCRIPTION;
        else if (0 ==pBegin->Name.compareToAscii(PROPERTY_TYPE))
            pBegin->Handle = PROPERTY_ID_TYPE;
    }

    describeProperties(aTableProps);

    return new ::cppu::OPropertyArrayHelper(aTableProps);
}
// -----------------------------------------------------------------------------
void ODBTableDecorator::setTable(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier >& _rxTable)
{
    m_xTable = _rxTable;
}
// -----------------------------------------------------------------------------
::cppu::IPropertyArrayHelper & SAL_CALL ODBTableDecorator::getInfoHelper()
{
    Reference<XPropertySet> xProp(m_xTable,UNO_QUERY);
    Reference<XPropertySetInfo> xInfo = xProp->getPropertySetInfo();

    return *ODBTableDecorator_PROP::getArrayHelper((xInfo->getPropertyByName(PROPERTY_NAME).Attributes & PropertyAttribute::READONLY) == PropertyAttribute::READONLY ? 1 : 0);
}
// -------------------------------------------------------------------------
// XServiceInfo
IMPLEMENT_SERVICE_INFO1(ODBTableDecorator, "com.sun.star.sdb.dbaccess.ODBTableDecorator", SERVICE_SDBCX_TABLE)
// -------------------------------------------------------------------------
Any SAL_CALL ODBTableDecorator::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet;
    if(m_xTable.is())
    {
        aRet = m_xTable->queryInterface(rType);
        if(aRet.hasValue())
        {   // now we know that our table supports this type so we return ourself
            aRet = OTableDescriptor_BASE::queryInterface(rType);
            if(!aRet.hasValue())
                aRet = ODataSettings::queryInterface(rType);
        }
    }
    if(!aRet.hasValue())
        aRet = OConfigurationFlushable::queryInterface( rType);

    return aRet;
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL ODBTableDecorator::getTypes(  ) throw(RuntimeException)
{
    Reference<XTypeProvider> xTypes(m_xTable,UNO_QUERY);
    OSL_ENSURE(xTypes.is(),"Table must be a TypePropvider!");
    return ::comphelper::concatSequences(xTypes->getTypes(),OConfigurationFlushable::getTypes());
}

// -----------------------------------------------------------------------------
void ODBTableDecorator::flush_NoBroadcast_NoCommit()
{
    if(m_aConfigurationNode.isValid())
    {
        storeTo(m_aConfigurationNode.openNode(CONFIGKEY_SETTINGS));

        OColumns* pColumns = static_cast<OColumns*>(m_pColumns);
        if(pColumns)
            pColumns->storeSettings( m_aConfigurationNode.openNode( CONFIGKEY_QRYDESCR_COLUMNS ), m_xNumberFormats );
    }
}
// XRename,
//------------------------------------------------------------------------------
void SAL_CALL ODBTableDecorator::rename( const ::rtl::OUString& _rNewName ) throw(SQLException, ElementExistException, RuntimeException)
{
    Reference<XRename> xRename(m_xTable,UNO_QUERY);
    if(xRename.is())
    {
        xRename->rename(_rNewName);
    }
    else // not supported
        throw SQLException(::rtl::OUString::createFromAscii("Driver does not support this function!"),*this,::rtl::OUString::createFromAscii("IM001"),0,Any());
}

// XAlterTable,
//------------------------------------------------------------------------------
void SAL_CALL ODBTableDecorator::alterColumnByName( const ::rtl::OUString& _rName, const Reference< XPropertySet >& _rxDescriptor ) throw(SQLException, NoSuchElementException, RuntimeException)
{
    Reference<XAlterTable> xAlter(m_xTable,UNO_QUERY);
    if(xAlter.is())
    {
        xAlter->alterColumnByName(_rName,_rxDescriptor);
    }
    else
        // not supported
        throw SQLException(::rtl::OUString::createFromAscii("Driver does not support this function!"),*this,::rtl::OUString::createFromAscii("IM001"),0,Any());
    m_pColumns->refresh();
}

//------------------------------------------------------------------------------
void SAL_CALL ODBTableDecorator::alterColumnByIndex( sal_Int32 _nIndex, const Reference< XPropertySet >& _rxDescriptor ) throw(SQLException, IndexOutOfBoundsException, RuntimeException)
{
    Reference<XAlterTable> xAlter(m_xTable,UNO_QUERY);
    if(xAlter.is())
    {
        xAlter->alterColumnByIndex(_nIndex,_rxDescriptor);
        m_pColumns->refresh();
    }
    else // not supported
        throw SQLException(::rtl::OUString::createFromAscii("Driver does not support this function!"),*this,::rtl::OUString::createFromAscii("IM001"),0,Any());
}
// -----------------------------------------------------------------------------
Reference< XNameAccess> ODBTableDecorator::getIndexes() throw (RuntimeException)
{
    return Reference< XIndexesSupplier>(m_xTable,UNO_QUERY)->getIndexes();
}
// -------------------------------------------------------------------------
Reference< XIndexAccess> ODBTableDecorator::getKeys() throw (RuntimeException)
{
    return Reference< XKeysSupplier>(m_xTable,UNO_QUERY)->getKeys();
}
// -------------------------------------------------------------------------
Reference< XNameAccess> ODBTableDecorator::getColumns() throw (RuntimeException)
{
    if(!m_pColumns)
    {
        ::std::vector< ::rtl::OUString> aVector;

        Reference<XNameAccess> xNames;
        if(m_xTable.is())
        {
            xNames = m_xTable->getColumns();
            if(xNames.is())
            {
                Sequence< ::rtl::OUString> aNames = xNames->getElementNames();
                const ::rtl::OUString* pBegin   = aNames.getConstArray();
                const ::rtl::OUString* pEnd     = pBegin + aNames.getLength();
                for(;pBegin != pEnd;++pBegin)
                    aVector.push_back(*pBegin);
            }
        }
        OColumns* pCol = new OColumns(*this,m_aMutex,xNames,m_xMetaData->storesMixedCaseQuotedIdentifiers(),aVector,
                                    this,this,
                                    m_xMetaData->supportsAlterTableWithAddColumn(),
                                    m_xMetaData->supportsAlterTableWithDropColumn());
        //  pCol->setParent(this);
        m_pColumns  = pCol;

        // load the UI settings of the columns
        if (m_aConfigurationNode.isValid())
            pCol->loadSettings( m_aConfigurationNode.openNode( CONFIGKEY_QRYDESCR_COLUMNS ), m_xNumberFormats );
    }

    return m_pColumns;
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODBTableDecorator::getName() throw(RuntimeException)
{
    Reference<XNamed> xName(m_xTable,UNO_QUERY);
    OSL_ENSURE(xName.is(),"Table should support the XNamed interface");
    return xName->getName();
}
// -----------------------------------------------------------------------------
sal_Int64 SAL_CALL ODBTableDecorator::getSomething( const Sequence< sal_Int8 >& rId ) throw(RuntimeException)
{
    if (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        return (sal_Int64)this;

    sal_Int64 nRet = 0;
    Reference<XUnoTunnel> xTunnel(m_xTable,UNO_QUERY);
    if(xTunnel.is())
        nRet = xTunnel->getSomething(rId);
    if(!nRet)
        nRet = OConfigurationFlushable::getSomething(rId);
    return nRet;
}
// -----------------------------------------------------------------------------
Sequence< sal_Int8 > ODBTableDecorator::getUnoTunnelImplementationId()
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
void ODBTableDecorator::fillPrivileges() const
{
    // somebody is asking for the privileges an we do not know them, yet
    const_cast<ODBTableDecorator*>(this)->m_nPrivileges = 0;    // don't allow anything if something goes wrong
    try
    {
        Reference<XPropertySet> xProp(m_xTable,UNO_QUERY);
        Any aVal = xProp->getPropertyValue(PROPERTY_CATALOGNAME);
        ::rtl::OUString sSchema,sName;
        xProp->getPropertyValue(PROPERTY_SCHEMANAME)    >>= sSchema;
        xProp->getPropertyValue(PROPERTY_NAME)          >>= sName;
        Reference< XResultSet > xPrivileges = m_xMetaData->getTablePrivileges(aVal,sSchema,sName);
        Reference< XRow > xCurrentRow(xPrivileges, UNO_QUERY);

        if (xCurrentRow.is())
        {
            ::rtl::OUString sUserWorkingFor = m_xMetaData->getUserName();
            // after creation the set is positioned before the first record, per definitionem

            ::rtl::OUString sPrivilege, sGrantee;
            while (xPrivileges->next())
            {
#ifdef DBG_UTIL
                ::rtl::OUString sCat, sSchema, sName, sGrantor, sGrantable;
                sCat        = xCurrentRow->getString(1);
                sSchema     = xCurrentRow->getString(2);
                sName       = xCurrentRow->getString(3);
                sGrantor    = xCurrentRow->getString(4);
#endif
                sGrantee    = xCurrentRow->getString(5);
                sPrivilege  = xCurrentRow->getString(6);
#ifdef DBG_UTIL
                sGrantable  = xCurrentRow->getString(7);
#endif

                if (sUserWorkingFor != sGrantee)
                    continue;

                if (sPrivilege.compareToAscii("SELECT") == 0)
                    const_cast<ODBTableDecorator*>(this)->m_nPrivileges |= Privilege::SELECT;
                else if (sPrivilege.compareToAscii("INSERT") == 0)
                    const_cast<ODBTableDecorator*>(this)->m_nPrivileges |= Privilege::INSERT;
                else if (sPrivilege.compareToAscii("UPDATE") == 0)
                    const_cast<ODBTableDecorator*>(this)->m_nPrivileges |= Privilege::UPDATE;
                else if (sPrivilege.compareToAscii("DELETE") == 0)
                    const_cast<ODBTableDecorator*>(this)->m_nPrivileges |= Privilege::DELETE;
                else if (sPrivilege.compareToAscii("READ") == 0)
                    const_cast<ODBTableDecorator*>(this)->m_nPrivileges |= Privilege::READ;
                else if (sPrivilege.compareToAscii("CREATE") == 0)
                    const_cast<ODBTableDecorator*>(this)->m_nPrivileges |= Privilege::CREATE;
                else if (sPrivilege.compareToAscii("ALTER") == 0)
                    const_cast<ODBTableDecorator*>(this)->m_nPrivileges |= Privilege::ALTER;
                else if (sPrivilege.compareToAscii("REFERENCE") == 0)
                    const_cast<ODBTableDecorator*>(this)->m_nPrivileges |= Privilege::REFERENCE;
                else if (sPrivilege.compareToAscii("DROP") == 0)
                    const_cast<ODBTableDecorator*>(this)->m_nPrivileges |= Privilege::DROP;
            }
        }
        disposeComponent(xPrivileges);
    }
    catch(SQLException& e)
    {
        UNUSED(e);
        DBG_ERROR("ODBTableDecorator::ODBTableDecorator : could not collect the privileges !");
    }
}
// -----------------------------------------------------------------------------
Reference< XPropertySet > SAL_CALL ODBTableDecorator::createDataDescriptor(  ) throw (RuntimeException)
{
    Reference< XDataDescriptorFactory > xFactory( m_xTable, UNO_QUERY );
    DBG_ASSERT( xFactory.is(), "ODBTableDecorator::createDataDescriptor: invalid table!" );
    Reference< XColumnsSupplier > xColsSupp;
    if ( xFactory.is() )
        xColsSupp = xColsSupp.query( xFactory->createDataDescriptor() );

    return new ODBTableDecorator(
        m_aConfigurationNode.cloneAsRoot(),
        m_xMetaData,
        xColsSupp,
        m_xNumberFormats
    );
}
// -----------------------------------------------------------------------------
Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL ODBTableDecorator::getPropertySetInfo(  ) throw(RuntimeException)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}
// -----------------------------------------------------------------------------
void ODBTableDecorator::refreshColumns()
{
    ::std::vector< ::rtl::OUString> aVector;

    if(m_xTable.is())
    {
        Reference<XNameAccess> xNames = m_xTable->getColumns();
        if(xNames.is())
        {
            Sequence< ::rtl::OUString> aNames = xNames->getElementNames();
            const ::rtl::OUString* pBegin   = aNames.getConstArray();
            const ::rtl::OUString* pEnd     = pBegin + aNames.getLength();
            for(;pBegin != pEnd;++pBegin)
                aVector.push_back(*pBegin);
        }
    }
    m_pColumns->reFill(aVector);
}
// -----------------------------------------------------------------------------
OColumn* ODBTableDecorator::createColumn(const ::rtl::OUString& _rName) const
{
    OColumn* pReturn = NULL;

    Reference< XNamed > xRet = NULL;
    Reference<XNameAccess> xNames;
    if(m_xTable.is())
        xNames = m_xTable->getColumns();
    if(xNames.is() && xNames->hasByName(_rName))
    {
        Reference<XPropertySet> xProp;
        xNames->getByName(_rName) >>= xProp;

        pReturn = new OTableColumnWrapper(xProp);
    }
    return pReturn;
}
// -----------------------------------------------------------------------------
Reference< XPropertySet > ODBTableDecorator::createEmptyObject()
{
    Reference<XDataDescriptorFactory> xNames;
    if(m_xTable.is())
        xNames = Reference<XDataDescriptorFactory>(m_xTable->getColumns(),UNO_QUERY);
    Reference< XPropertySet > xRet;
    if(xNames.is())
        xRet = new OTableColumnDescriptorWrapper(xNames->createDataDescriptor());
    return xRet;
}
// -----------------------------------------------------------------------------
void SAL_CALL ODBTableDecorator::acquire() throw(::com::sun::star::uno::RuntimeException)
{
    OTableDescriptor_BASE::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL ODBTableDecorator::release() throw(::com::sun::star::uno::RuntimeException)
{
    OTableDescriptor_BASE::release();
}

// -----------------------------------------------------------------------------
void SAL_CALL ODBTableDecorator::setName( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException)
{
}

// -----------------------------------------------------------------------------
void ODBTableDecorator::setContext( const ::utl::OConfigurationTreeRoot& _rNode, const Reference< XNumberFormatsSupplier >& _rxNumberFormats )
{
    OConfigurationFlushable::setConfigurationNode( _rNode );
    m_xNumberFormats = _rxNumberFormats;
}

