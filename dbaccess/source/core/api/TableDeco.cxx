/*************************************************************************
 *
 *  $RCSfile: TableDeco.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:01:48 $
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
#ifndef _DBA_CORE_RESOURCE_HXX_
#include "core_resource.hxx"
#endif
#ifndef _DBA_CORE_RESOURCE_HRC_
#include "core_resource.hrc"
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
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef DBA_CONTAINERMEDIATOR_HXX
#include "ContainerMediator.hxx"
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
using namespace ::dbtools;
using namespace ::cppu;

//==========================================================================
//= ODBTableDecorator
//==========================================================================
DBG_NAME(ODBTableDecorator)
// -----------------------------------------------------------------------------
ODBTableDecorator::ODBTableDecorator(
         const Reference< XDatabaseMetaData >& _rxMetaData
        ,const Reference< XColumnsSupplier >& _rxNewTable
        ,const Reference< XNumberFormatsSupplier >& _rxNumberFormats
        ,const Reference< XNameAccess >& _xColumnDefinitions
        ) throw(SQLException)
    :OTableDescriptor_BASE(m_aMutex)
    ,ODataSettings(OTableDescriptor_BASE::rBHelper)
    ,m_nPrivileges(-1)
    ,m_xMetaData(_rxMetaData)
    ,m_xTable(_rxNewTable)
    ,m_xNumberFormats( _rxNumberFormats )
    ,m_xColumnDefinitions(_xColumnDefinitions)
    ,m_pColumns(NULL)
{
    DBG_CTOR(ODBTableDecorator, NULL);
    ODataSettings::registerProperties(this);
}
// -------------------------------------------------------------------------
ODBTableDecorator::~ODBTableDecorator()
{
    DBG_DTOR(ODBTableDecorator, NULL);
    if ( m_pColumns )
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
    OPropertySetHelper::disposing();
    OTableDescriptor_BASE::disposing();

    MutexGuard aGuard(m_aMutex);
    m_xTable        = NULL;
    m_xMetaData     = NULL;
    m_pTables       = NULL;
    m_xColumnDefinitions = NULL;
    m_xNumberFormats = NULL;
    m_xColumnMediator = NULL;
    if ( m_pColumns )
    {
        m_pColumns->disposing();
    }
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
        case PROPERTY_ID_FONTCHARWIDTH:
        case PROPERTY_ID_FONTCHARSET:
        case PROPERTY_ID_FONTFAMILY:
        case PROPERTY_ID_FONTHEIGHT:
        case PROPERTY_ID_FONTKERNING:
        case PROPERTY_ID_FONTNAME:
        case PROPERTY_ID_FONTORIENTATION:
        case PROPERTY_ID_FONTPITCH:
        case PROPERTY_ID_FONTSLANT:
        case PROPERTY_ID_FONTSTRIKEOUT:
        case PROPERTY_ID_FONTSTYLENAME:
        case PROPERTY_ID_FONTUNDERLINE:
        case PROPERTY_ID_FONTWEIGHT:
        case PROPERTY_ID_FONTWIDTH:
        case PROPERTY_ID_FONTWORDLINEMODE:
        case PROPERTY_ID_FONTTYPE:
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
        case PROPERTY_ID_FONTCHARWIDTH:
        case PROPERTY_ID_FONTCHARSET:
        case PROPERTY_ID_FONTFAMILY:
        case PROPERTY_ID_FONTHEIGHT:
        case PROPERTY_ID_FONTKERNING:
        case PROPERTY_ID_FONTNAME:
        case PROPERTY_ID_FONTORIENTATION:
        case PROPERTY_ID_FONTPITCH:
        case PROPERTY_ID_FONTSLANT:
        case PROPERTY_ID_FONTSTRIKEOUT:
        case PROPERTY_ID_FONTSTYLENAME:
        case PROPERTY_ID_FONTUNDERLINE:
        case PROPERTY_ID_FONTWEIGHT:
        case PROPERTY_ID_FONTWIDTH:
        case PROPERTY_ID_FONTWORDLINEMODE:
        case PROPERTY_ID_FONTTYPE:

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
            {
                if ( -1 == m_nPrivileges )
                    fillPrivileges();
                Reference<XPropertySet> xProp(m_xTable,UNO_QUERY);
                Reference<XPropertySetInfo> xInfo = xProp->getPropertySetInfo();
                if ( xInfo->hasPropertyByName(PROPERTY_PRIVILEGES) )
                {
                    _rValue <<= m_nPrivileges;
                    break;
                }
            }
            // run through

        case PROPERTY_ID_FILTER:
        case PROPERTY_ID_ORDER:
        case PROPERTY_ID_APPLYFILTER:
        case PROPERTY_ID_FONT:
        case PROPERTY_ID_ROW_HEIGHT:
        case PROPERTY_ID_TEXTCOLOR:
        case PROPERTY_ID_TEXTLINECOLOR:
        case PROPERTY_ID_TEXTEMPHASIS:
        case PROPERTY_ID_TEXTRELIEF:
        case PROPERTY_ID_FONTCHARWIDTH:
        case PROPERTY_ID_FONTCHARSET:
        case PROPERTY_ID_FONTFAMILY:
        case PROPERTY_ID_FONTHEIGHT:
        case PROPERTY_ID_FONTKERNING:
        case PROPERTY_ID_FONTNAME:
        case PROPERTY_ID_FONTORIENTATION:
        case PROPERTY_ID_FONTPITCH:
        case PROPERTY_ID_FONTSLANT:
        case PROPERTY_ID_FONTSTRIKEOUT:
        case PROPERTY_ID_FONTSTYLENAME:
        case PROPERTY_ID_FONTUNDERLINE:
        case PROPERTY_ID_FONTWEIGHT:
        case PROPERTY_ID_FONTWIDTH:
        case PROPERTY_ID_FONTWORDLINEMODE:
        case PROPERTY_ID_FONTTYPE:
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
    sal_Bool bNotFound = sal_True;
    Reference<XPropertySet> xProp(m_xTable,UNO_QUERY);
    if ( xProp.is() )
    {
        Reference<XPropertySetInfo> xInfo = xProp->getPropertySetInfo();
        bNotFound = !xInfo->hasPropertyByName(PROPERTY_PRIVILEGES);
    }
    if ( bNotFound )
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
        else if (0 ==pBegin->Name.compareToAscii(PROPERTY_PRIVILEGES))
            pBegin->Handle = PROPERTY_ID_PRIVILEGES;
    }

    describeProperties(aTableProps);

    return new ::cppu::OPropertyArrayHelper(aTableProps);
}
// -----------------------------------------------------------------------------
void ODBTableDecorator::setTable(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier >& _rxTable)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OTableDescriptor_BASE::rBHelper.bDisposed);
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

    return aRet;
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL ODBTableDecorator::getTypes(  ) throw(RuntimeException)
{
    Reference<XTypeProvider> xTypes(m_xTable,UNO_QUERY);
    OSL_ENSURE(xTypes.is(),"Table must be a TypePropvider!");
    return xTypes->getTypes();
}

// -----------------------------------------------------------------------------
// XRename,
//------------------------------------------------------------------------------
void SAL_CALL ODBTableDecorator::rename( const ::rtl::OUString& _rNewName ) throw(SQLException, ElementExistException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OTableDescriptor_BASE::rBHelper.bDisposed);
    Reference<XRename> xRename(m_xTable,UNO_QUERY);
    if(xRename.is())
    {
//      ::rtl::OUString sOldName;
//      Reference<XPropertySet> xProp(m_xTable,UNO_QUERY);
//      xProp->getPropertyValue(PROPERTY_NAME) >>= sOldName;
        xRename->rename(_rNewName);
    }
    else // not supported
        throw SQLException(DBACORE_RESSTRING(RID_STR_NO_TABLE_RENAME),*this,SQLSTATE_GENERAL,1000,Any() );
}

// XAlterTable,
//------------------------------------------------------------------------------
void SAL_CALL ODBTableDecorator::alterColumnByName( const ::rtl::OUString& _rName, const Reference< XPropertySet >& _rxDescriptor ) throw(SQLException, NoSuchElementException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OTableDescriptor_BASE::rBHelper.bDisposed);
    Reference<XAlterTable> xAlter(m_xTable,UNO_QUERY);
    if(xAlter.is())
    {
        xAlter->alterColumnByName(_rName,_rxDescriptor);
    }
    else
        throw SQLException(DBACORE_RESSTRING(RID_STR_COLUMN_ALTER_BY_NAME),*this,SQLSTATE_GENERAL,1000,Any() );
    if(m_pColumns)
        m_pColumns->refresh();
}

//------------------------------------------------------------------------------
void SAL_CALL ODBTableDecorator::alterColumnByIndex( sal_Int32 _nIndex, const Reference< XPropertySet >& _rxDescriptor ) throw(SQLException, IndexOutOfBoundsException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OTableDescriptor_BASE::rBHelper.bDisposed);
    Reference<XAlterTable> xAlter(m_xTable,UNO_QUERY);
    if(xAlter.is())
    {
        xAlter->alterColumnByIndex(_nIndex,_rxDescriptor);
        if(m_pColumns)
            m_pColumns->refresh();
    }
    else // not supported
        throw SQLException(DBACORE_RESSTRING(RID_STR_COLUMN_ALTER_BY_INDEX),*this,SQLSTATE_GENERAL,1000,Any() );
}
// -----------------------------------------------------------------------------
Reference< XNameAccess> ODBTableDecorator::getIndexes() throw (RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OTableDescriptor_BASE::rBHelper.bDisposed);
    return Reference< XIndexesSupplier>(m_xTable,UNO_QUERY)->getIndexes();
}
// -------------------------------------------------------------------------
Reference< XIndexAccess> ODBTableDecorator::getKeys() throw (RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OTableDescriptor_BASE::rBHelper.bDisposed);
    return Reference< XKeysSupplier>(m_xTable,UNO_QUERY)->getKeys();
}
// -------------------------------------------------------------------------
Reference< XNameAccess> ODBTableDecorator::getColumns() throw (RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OTableDescriptor_BASE::rBHelper.bDisposed);

    if(!m_pColumns)
        refreshColumns();

    return m_pColumns;
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODBTableDecorator::getName() throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OTableDescriptor_BASE::rBHelper.bDisposed);
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
    m_nPrivileges = 0;
    try
    {
        Reference<XPropertySet> xProp(m_xTable,UNO_QUERY);
        if ( xProp.is() )
        {
            if ( xProp->getPropertySetInfo()->hasPropertyByName(PROPERTY_PRIVILEGES) )
            {
                xProp->getPropertyValue(PROPERTY_PRIVILEGES) >>= m_nPrivileges;
            }
            if ( m_nPrivileges == 0 ) // second chance
            {
                ::rtl::OUString sCatalog,sSchema,sName;
                xProp->getPropertyValue(PROPERTY_CATALOGNAME)   >>= sCatalog;
                xProp->getPropertyValue(PROPERTY_SCHEMANAME)    >>= sSchema;
                xProp->getPropertyValue(PROPERTY_NAME)          >>= sName;
                m_nPrivileges = ::dbtools::getTablePrivileges(getMetaData(),sCatalog,sSchema, sName);
            }
        }
    }
    catch(const SQLException& e)
    {
        UNUSED(e);
        DBG_ERROR("ODBTableDecorator::ODBTableDecorator : could not collect the privileges !");
    }
}
// -----------------------------------------------------------------------------
Reference< XPropertySet > SAL_CALL ODBTableDecorator::createDataDescriptor(  ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OTableDescriptor_BASE::rBHelper.bDisposed);

    Reference< XDataDescriptorFactory > xFactory( m_xTable, UNO_QUERY );
    DBG_ASSERT( xFactory.is(), "ODBTableDecorator::createDataDescriptor: invalid table!" );
    Reference< XColumnsSupplier > xColsSupp;
    if ( xFactory.is() )
        xColsSupp = xColsSupp.query( xFactory->createDataDescriptor() );

    return new ODBTableDecorator(
        m_xMetaData,
        xColsSupp,
        m_xNumberFormats,
        NULL
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
    ::osl::MutexGuard aGuard(m_aMutex);
    ::connectivity::checkDisposed(OTableDescriptor_BASE::rBHelper.bDisposed);

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
    if(!m_pColumns)
    {
        OColumns* pCol = new OColumns(*this,m_aMutex,xNames,m_xMetaData.is() && m_xMetaData->storesMixedCaseQuotedIdentifiers(),aVector,
                                    this,this,
                                    m_xMetaData.is() && m_xMetaData->supportsAlterTableWithAddColumn(),
                                    m_xMetaData.is() && m_xMetaData->supportsAlterTableWithDropColumn());

        OContainerMediator* pMediator = new OContainerMediator(pCol,m_xColumnDefinitions,sal_False);
        m_xColumnMediator = pMediator;
        pCol->setMediator(pMediator);
        m_pColumns  = pCol;
    }
    else
        m_pColumns->reFill(aVector);
}
// -----------------------------------------------------------------------------
OColumn* ODBTableDecorator::createColumn(const ::rtl::OUString& _rName) const
{
    OColumn* pReturn = NULL;

    Reference<XNameAccess> xNames;
    if ( m_xTable.is() )
    {
        xNames = m_xTable->getColumns();

        if ( xNames.is() && xNames->hasByName(_rName) )
        {
            Reference<XPropertySet> xProp(xNames->getByName(_rName),UNO_QUERY);

            Reference<XPropertySet> xColumnDefintion;
            if ( m_xColumnDefinitions.is() && m_xColumnDefinitions->hasByName(_rName))
                xColumnDefintion.set(m_xColumnDefinitions->getByName(_rName),UNO_QUERY);

            pReturn = new OTableColumnWrapper(xProp,xColumnDefintion);
        }
    }
    return pReturn;
}
// -----------------------------------------------------------------------------
void ODBTableDecorator::columnDropped(const ::rtl::OUString& _sName)
{
    Reference<XDrop> xDrop(m_xColumnDefinitions,UNO_QUERY);
    if ( xDrop.is() && m_xColumnDefinitions->hasByName(_sName) )
    {
        xDrop->dropByName(_sName);
    }
}
// -----------------------------------------------------------------------------
Reference< XPropertySet > ODBTableDecorator::createEmptyObject()
{
    Reference<XDataDescriptorFactory> xNames;
    if(m_xTable.is())
        xNames.set(m_xTable->getColumns(),UNO_QUERY);
    Reference< XPropertySet > xRet;
    if(xNames.is())
        xRet = new OTableColumnDescriptorWrapper(xNames->createDataDescriptor());
    return xRet;
}
// -----------------------------------------------------------------------------
void SAL_CALL ODBTableDecorator::acquire() throw()
{
    OTableDescriptor_BASE::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL ODBTableDecorator::release() throw()
{
    OTableDescriptor_BASE::release();
}

// -----------------------------------------------------------------------------
void SAL_CALL ODBTableDecorator::setName( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException)
{
}

// -----------------------------------------------------------------------------


