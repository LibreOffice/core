/*************************************************************************
 *
 *  $RCSfile: column.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: fs $ $Date: 2001-04-06 08:58:10 $
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

#ifndef _DBA_COREAPI_COLUMN_HXX_
#include "column.hxx"
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef _DBA_CORE_REGISTRYHELPER_HXX_
#include "registryhelper.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_ENUMHELPER_HXX_
#include <comphelper/enumhelper.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _COMPHELPER_SEQSTREAM_HXX
#include <comphelper/seqstream.hxx>
#endif
#ifndef _COMPHELPER_BASIC_IO_HXX_
#include <comphelper/basicio.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _DBA_CORE_TABLE_HXX_
#include "table.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_COLUMN_HXX_
#include <connectivity/sdbcx/VColumn.hxx>
#endif
#ifndef _DBACORE_DEFINITIONCOLUMN_HXX_
#include "definitioncolumn.hxx"
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif

using namespace dbaccess;
using namespace connectivity;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::registry;
using namespace ::osl;
using namespace ::comphelper;
using namespace ::cppu;

DBG_NAME(OColumn);

#define COLUMN_STREAM_SIGNATURE ::rtl::OUString::createFromAscii("Columns")

//============================================================
//= OColumn
//============================================================
//--------------------------------------------------------------------------
OColumn::OColumn()
        :OColumnBase(m_aMutex)
        , OPropertySetHelper(OColumnBase::rBHelper)
{
    DBG_CTOR(OColumn, NULL);
}

//--------------------------------------------------------------------------
OColumn::~OColumn()
{
    DBG_DTOR(OColumn, NULL);
}

// com::sun::star::lang::XTypeProvider
//--------------------------------------------------------------------------
Sequence< Type > OColumn::getTypes() throw (RuntimeException)
{
    OTypeCollection aTypes(::getCppuType( (const Reference< XPropertySet > *)0 ),
                           ::getCppuType( (const Reference< XMultiPropertySet > *)0 ),
                           OColumnBase::getTypes());
    return aTypes.getTypes();
}

// com::sun::star::uno::XInterface
//--------------------------------------------------------------------------
Any OColumn::queryInterface( const Type & rType ) throw (RuntimeException)
{
    Any aIface = OColumnBase::queryInterface( rType );
    if (!aIface.hasValue())
        aIface = ::cppu::queryInterface(
                    rType,
                    static_cast< XPropertySet * >( this ),
                    static_cast< XMultiPropertySet * >( this ));

    return aIface;
}

//--------------------------------------------------------------------------
void OColumn::acquire() throw(RuntimeException)
{
    OColumnBase::acquire();
}

//--------------------------------------------------------------------------
void OColumn::release() throw(RuntimeException)
{
    OColumnBase::release();
}

// ::com::sun::star::lang::XServiceInfo
//------------------------------------------------------------------------------
rtl::OUString OColumn::getImplementationName(  ) throw(RuntimeException)
{
    return rtl::OUString::createFromAscii("com.sun.star.sdb.OColumn");
}

//------------------------------------------------------------------------------
sal_Bool OColumn::supportsService( const ::rtl::OUString& _rServiceName ) throw (RuntimeException)
{
    return ::comphelper::findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > OColumn::getSupportedServiceNames(  ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aSNS( 1 );
    aSNS[0] = SERVICE_SDBCX_COLUMN;
    return aSNS;
}

// OComponentHelper
//------------------------------------------------------------------------------
void OColumn::disposing()
{
    OPropertySetHelper::disposing();
}

// com::sun::star::beans::XPropertySet
//------------------------------------------------------------------------------
Reference< XPropertySetInfo > OColumn::getPropertySetInfo() throw (RuntimeException)
{
    return createPropertySetInfo( getInfoHelper() ) ;
}

//------------------------------------------------------------------------------
void OColumn::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_NAME:
            rValue <<= m_sName;
            break;
    }
}

//------------------------------------------------------------------------------
sal_Bool OColumn::convertFastPropertyValue(
                            Any & rConvertedValue,
                            Any & rOldValue,
                            sal_Int32 nHandle,
                            const Any& rValue )
                                throw (IllegalArgumentException)
{
    sal_Bool bModified = sal_False;
    switch (nHandle)
    {
        case PROPERTY_ID_NAME:
            bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_sName);
            break;
    }
    return bModified;
}

//------------------------------------------------------------------------------
void OColumn::setFastPropertyValue_NoBroadcast(
                                sal_Int32 nHandle,
                                const Any& rValue
                                                 )
                                                 throw (Exception)
{
    switch (nHandle)
    {
        case PROPERTY_ID_NAME:
            OSL_ENSURE(rValue.getValueType().equals(::getCppuType(static_cast< ::rtl::OUString* >(NULL))),
                "OColumn::setFastPropertyValue_NoBroadcast(NAME) : invalid value !");
            rValue >>= m_sName;
            break;
    }
}

//--------------------------------------------------------------------------
Sequence< sal_Int8 > OColumn::getUnoTunnelImplementationId()
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

// com::sun::star::lang::XUnoTunnel
//------------------------------------------------------------------
sal_Int64 OColumn::getSomething( const Sequence< sal_Int8 > & rId ) throw (RuntimeException)
{
    if (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        return (sal_Int64)this;

    return 0;
}

//============================================================
//= OColumnSettings
//============================================================
//------------------------------------------------------------------------------
void OColumnSettings::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_ALIGN:
            rValue = m_aAlignment;
            break;
        case PROPERTY_ID_NUMBERFORMAT:
            rValue = m_aFormatKey;
            break;
        case PROPERTY_ID_RELATIVEPOSITION:
            rValue = m_aRelativePosition;
            break;
        case PROPERTY_ID_WIDTH:
            rValue = m_aWidth;
            break;
        case PROPERTY_ID_HIDDEN:
            rValue.setValue(&m_bHidden, getBooleanCppuType());
            break;
        case PROPERTY_ID_CONTROLMODEL:
            rValue <<= m_xControlModel;
            break;
    }
}

//------------------------------------------------------------------------------
sal_Bool OColumnSettings::convertFastPropertyValue(
                            Any & rConvertedValue,
                            Any & rOldValue,
                            sal_Int32 nHandle,
                            const Any& rValue )
                                throw (IllegalArgumentException)
{
    sal_Bool bModified = sal_False;
    switch (nHandle)
    {
        case PROPERTY_ID_ALIGN:
            bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aAlignment,
                ::getCppuType(static_cast< sal_Int32* >(NULL)));
            break;
        case PROPERTY_ID_WIDTH:
            bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aWidth,
                ::getCppuType(static_cast< sal_Int32* >(NULL)));
            break;
        case PROPERTY_ID_HIDDEN:
            bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_bHidden);
            break;
        case PROPERTY_ID_RELATIVEPOSITION:
            bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aRelativePosition,
                ::getCppuType(static_cast< sal_Int32* >(NULL)));
            break;
        case PROPERTY_ID_NUMBERFORMAT:
            bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aFormatKey,
                ::getCppuType(static_cast< sal_Int32* >(NULL)));
            break;
        case PROPERTY_ID_CONTROLMODEL:
        {
            Reference< XPropertySet > xTest;
            if (!::cppu::extractInterface(xTest, rValue))
                throw IllegalArgumentException();
            if (xTest.get() != m_xControlModel.get())
            {
                bModified = sal_True;
                rOldValue <<= m_xControlModel;
                rConvertedValue <<= rValue;
            }
        }
        break;
    }
    return bModified;
}

//------------------------------------------------------------------------------
void OColumnSettings::setFastPropertyValue_NoBroadcast(
                                            sal_Int32 nHandle,
                                            const Any& rValue
                                                 )
                                                 throw (Exception)
{
    switch (nHandle)
    {
        case PROPERTY_ID_ALIGN:
            OSL_ENSURE(!rValue.hasValue() || rValue.getValueType().equals(::getCppuType(static_cast< sal_Int32* >(NULL))),
                "OColumnSettings::setFastPropertyValue_NoBroadcast(ALIGN) : invalid value !");
            m_aAlignment = rValue;
            break;
        case PROPERTY_ID_WIDTH:
            OSL_ENSURE(!rValue.hasValue() || rValue.getValueType().equals(::getCppuType(static_cast< sal_Int32* >(NULL))),
                "OColumnSettings::setFastPropertyValue_NoBroadcast(WIDTH) : invalid value !");
            m_aWidth = rValue;
            break;
        case PROPERTY_ID_NUMBERFORMAT:
            OSL_ENSURE(!rValue.hasValue() || rValue.getValueType().equals(::getCppuType(static_cast< sal_Int32* >(NULL))),
                "OColumnSettings::setFastPropertyValue_NoBroadcast(NUMBERFORMAT) : invalid value !");
            m_aFormatKey = rValue;
            break;
        case PROPERTY_ID_RELATIVEPOSITION:
            OSL_ENSURE(!rValue.hasValue() || rValue.getValueType().equals(::getCppuType(static_cast< sal_Int32* >(NULL))),
                "OColumnSettings::setFastPropertyValue_NoBroadcast(ID_RELATIVEPOSITION) : invalid value !");
            m_aWidth = rValue;
            break;
        case PROPERTY_ID_HIDDEN:
            OSL_ENSURE(rValue.getValueType().equals(::getBooleanCppuType()),
                "OColumnSettings::setFastPropertyValue_NoBroadcast(HIDDEN) : invalid value !");
            m_bHidden = ::comphelper::getBOOL(rValue);
            break;
    }
}


//------------------------------------------------------------------------------
sal_Bool OColumnSettings::writeUITo(const OConfigurationNode& _rConfigNode)
{
    _rConfigNode.setNodeValue(CONFIGKEY_COLUMN_ALIGNMENT, m_aAlignment);
    _rConfigNode.setNodeValue(CONFIGKEY_COLUMN_WIDTH, m_aWidth);
    _rConfigNode.setNodeValue(CONFIGKEY_COLUMN_NUMBERFORMAT, m_aFormatKey);
    _rConfigNode.setNodeValue(CONFIGKEY_COLUMN_RELPOSITION, m_aRelativePosition);
    _rConfigNode.setNodeValue(CONFIGKEY_COLUMN_HIDDEN, ::cppu::bool2any(m_bHidden));
    return sal_True;
}

//------------------------------------------------------------------------------
void OColumnSettings::readUIFrom(const OConfigurationNode& _rConfigNode)
{
    // some defaults
    m_bHidden = sal_False;
    m_aRelativePosition.clear();
    m_aFormatKey.clear();
    m_aWidth.clear();
    m_aAlignment.clear();

    m_aAlignment        = _rConfigNode.getNodeValue(CONFIGKEY_COLUMN_ALIGNMENT);
    m_aWidth            = _rConfigNode.getNodeValue(CONFIGKEY_COLUMN_WIDTH);
    m_aFormatKey        = _rConfigNode.getNodeValue(CONFIGKEY_COLUMN_NUMBERFORMAT);
    m_aRelativePosition = _rConfigNode.getNodeValue(CONFIGKEY_COLUMN_RELPOSITION);
    m_bHidden           = ::cppu::any2bool(_rConfigNode.getNodeValue(CONFIGKEY_COLUMN_HIDDEN));
}

//============================================================
//= OColumns
//============================================================
DBG_NAME(OColumns);

//--------------------------------------------------------------------------
OColumns::OColumns(::cppu::OWeakObject& _rParent,
                   ::osl::Mutex& _rMutex,
                   sal_Bool _bCaseSensitive,const ::std::vector< ::rtl::OUString> &_rVector, IColumnFactory* _pColFactory,
                   sal_Bool _bAddColumn,sal_Bool _bDropColumn)
                   : connectivity::sdbcx::OCollection(_rParent,_bCaseSensitive,_rMutex,_rVector)
    ,m_pTable(NULL)
    ,m_bInitialized(sal_False)
    ,m_bAddColumn(_bAddColumn)
    ,m_bDropColumn(_bDropColumn)
    ,m_xDrvColumns(NULL)
    ,m_pColFactoryImpl(_pColFactory)
{
    //  m_pColMap = new OColumnMap(17, ::utl::UStringMixHash(_bCaseSensitive), ::utl::UStringMixEqual(_bCaseSensitive));
    DBG_CTOR(OColumns, NULL);
}
// -------------------------------------------------------------------------
OColumns::OColumns(::cppu::OWeakObject& _rParent, ::osl::Mutex& _rMutex,
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxDrvColumns,
        sal_Bool _bCaseSensitive,const ::std::vector< ::rtl::OUString> &_rVector, IColumnFactory* _pColFactory,
        sal_Bool _bAddColumn,sal_Bool _bDropColumn)
       : connectivity::sdbcx::OCollection(_rParent,_bCaseSensitive,_rMutex,_rVector)
    ,m_pTable(NULL)
    ,m_bInitialized(sal_False)
    ,m_bAddColumn(_bAddColumn)
    ,m_bDropColumn(_bDropColumn)
    ,m_xDrvColumns(_rxDrvColumns)
    ,m_pColFactoryImpl(_pColFactory)
{
    DBG_CTOR(OColumns, NULL);
}
//--------------------------------------------------------------------------
OColumns::~OColumns()
{
    DBG_DTOR(OColumns, NULL);
    //  DELETEZ(m_pColMap);
}

// XServiceInfo
//------------------------------------------------------------------------------
rtl::OUString OColumns::getImplementationName(  ) throw(RuntimeException)
{
    return rtl::OUString::createFromAscii("com.sun.star.sdb.OColumns");
}

//------------------------------------------------------------------------------
sal_Bool OColumns::supportsService( const ::rtl::OUString& _rServiceName ) throw (RuntimeException)
{
    return ::comphelper::findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > OColumns::getSupportedServiceNames(  ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aSNS( 1 );
    aSNS[0] = SERVICE_SDBCX_CONTAINER;
    return aSNS;
}


//------------------------------------------------------------------
void OColumns::append(const ::rtl::OUString& rName, OColumn* pCol)
{
    MutexGuard aGuard(m_rMutex);
    pCol->acquire();
    pCol->m_sName = rName;
    ObjectIter aIter = m_aNameMap.find(rName);
    OSL_ENSURE(aIter == m_aNameMap.end(),"OColumns::append: Column already exists");

    m_aElements.push_back(m_aNameMap.insert(m_aNameMap.begin(), ObjectMap::value_type(rName,::com::sun::star::uno::WeakReference< ::com::sun::star::container::XNamed >(pCol))));

    //  (*m_pColMap)[rName] = pCol;
    //  m_aColArray.push_back(pCol);
}

//------------------------------------------------------------------
void OColumns::clearColumns()
{
    MutexGuard aGuard(m_rMutex);
    disposing();
}
// -----------------------------------------------------------------------------
void SAL_CALL OColumns::disposing(void)
{
    MutexGuard aGuard(m_rMutex);
    m_xDrvColumns = NULL;
    OColumns_BASE::disposing();
}
//------------------------------------------------------------------------------
void OColumns::loadSettings(const OConfigurationNode& _rLocation)
{
    MutexGuard aGuard(m_rMutex);

    OConfigurationNode aLocation(_rLocation);
    aLocation.setEscape(aLocation.isSetNode());

    OSL_ENSURE(m_pColFactoryImpl, "OColumns::loadSettings: need a factory to create columns!");

    Sequence< ::rtl::OUString > aColumNames = aLocation.getNodeNames();
    const ::rtl::OUString* pColumNames = aColumNames.getConstArray();
    for (sal_Int32 i=0; i<aColumNames.getLength(); ++i, ++pColumNames)
    {
        // do we already have a column with that name ?
        OColumn* pExistent = NULL;
        // create the column if neccessary
        if (!hasByName(*pColumNames))
        {
            if (m_pColFactoryImpl)
                pExistent = m_pColFactoryImpl->createColumn(*pColumNames);

            if (pExistent)
                append(*pColumNames, pExistent);
            else
            {
                OSL_ENSURE(sal_False, "OColumns::loadSettings : createColumn returned nonsense !");
                continue;
            }
        }
        else
        {
            Reference<XNamed> xColumn;
            ::cppu::extractInterface(xColumn,getByName(*pColumNames));
            Reference< ::com::sun::star::lang::XUnoTunnel> xTunnel(xColumn,UNO_QUERY);
            if(xTunnel.is())
                pExistent = (OColumn*)xTunnel->getSomething(OColumn::getUnoTunnelImplementationId());
            OSL_ENSURE(pExistent,"OColumns::loadSettings: No column from unotunnelhelper!");
        }

        OConfigurationNode aCurrent = aLocation.openNode(*pColumNames);
        OColumnSettings* pExistentSettings = pExistent->getSettings();
        if (pExistentSettings)
            pExistentSettings->readUIFrom(aCurrent);
        else
            OSL_ENSURE(sal_False, "OColumns::loadSettings : no settings for the column !");
    }
}

//------------------------------------------------------------------------------
void OColumns::storeSettings(const OConfigurationNode& _rLocation)
{
    MutexGuard aGuard(m_rMutex);
    if (!_rLocation.isValid())
    {
        OSL_ENSURE(sal_False, "OColumns::storeSettings: have no location !");
        return;
    }
    if (_rLocation.isReadonly())
    {
        OSL_ENSURE(sal_False, "OColumns::storeSettings: the location is read-only !");
        return;
    }

    DECLARE_STL_USTRINGACCESS_MAP(OConfigurationNode, MapName2Node);
    MapName2Node aObjectKeys;

    // collect the sub keys of existent column descriptions
    Sequence< ::rtl::OUString > aColumNames = _rLocation.getNodeNames();
    const ::rtl::OUString* pColumNames = aColumNames.getConstArray();
    for (sal_Int32 i=0; i<aColumNames.getLength(); ++i, ++pColumNames)
        aObjectKeys[*pColumNames] = _rLocation.openNode(*pColumNames);

    // now write all descriptions of my columns
    OColumn* pCurrent = NULL;
    ::rtl::OUString sCurrent;
    for (   ::std::vector< ObjectIter >::const_iterator aIter = m_aElements.begin();
            aIter != m_aElements.end();
            ++aIter
        )
    {
        // set the name
        Reference< ::com::sun::star::lang::XUnoTunnel> xTunnel((*aIter)->second.get(),UNO_QUERY);
        if(xTunnel.is())
        {
            pCurrent = (OColumn*)xTunnel->getSomething(OColumn::getUnoTunnelImplementationId());

            OSL_ENSURE(pCurrent,"OColumns::storeSettings: No column from unotunnelhelper!");

            OColumnSettings* pCurrentSettings = pCurrent->getSettings();
            if (!pCurrentSettings)
            {
                OSL_ENSURE(sal_False, "OColumns::storeSettings: can't write column without settings!");
                continue;
            }
            sCurrent = pCurrent->m_sName;

            OConfigurationNode aColumnNode;
            // do we we have an existent key for that column ?
            ConstMapName2NodeIterator aExistentObjectKey = aObjectKeys.find(sCurrent);
            if (aExistentObjectKey != aObjectKeys.end())
            {
                aColumnNode = aExistentObjectKey->second;
                // these sub key is used (and must not be deleted afterwards)
                // -> remove from the key maps
                aObjectKeys.erase(sCurrent);
            }
            else
            {   // no -> create one

                // the configuration does not support different types of operations in one transaction, so we must commit
                // before and after we create the new node, to ensure, that every transaction we ever do contains only
                // one type of operation (insert, remove, update)
                aColumnNode = _rLocation.createNode(sCurrent);
                if (!aColumnNode.isValid())
                {
                    OSL_ENSURE(sal_False, "OColumns::storeSettings: could not create the structures for writing a column !");
                    continue;
                }
            }

            // let the column write itself
            pCurrentSettings->writeUITo(aColumnNode);
        }
    }

    // delete all description keys where we have no columns for
    for (   ConstMapName2NodeIterator   aRemove = aObjectKeys.begin();
            aRemove != aObjectKeys.end();
            ++aRemove
        )
    {
        // the configuration does not support different types of operations in one transaction, so we must commit
        // before and after we create the new node, to ensure, that every transaction we ever do contains only
        // one type of operation (insert, remove, update)
        _rLocation.removeNode(aRemove->first);
    }
}

// -------------------------------------------------------------------------
void OColumns::impl_refresh() throw(::com::sun::star::uno::RuntimeException)
{
}

// -------------------------------------------------------------------------
Reference< XNamed > OColumns::createObject(const ::rtl::OUString& _rName)
{
    OSL_ENSURE(m_pColFactoryImpl, "OColumns::createObject: no column factory!");

    if (m_pColFactoryImpl)
        return m_pColFactoryImpl->createColumn(_rName);
    else
        return Reference< XNamed >();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OColumns::createEmptyObject()
{
    OSL_ENSURE(sal_False, "Are not filled this way!");
    //  connectivity::sdbcx::OColumn* pRet = new OTableColumnDescriptor(isCaseSensitive());
    OTableColumnDescriptor* pRet = new OTableColumnDescriptor();
    Reference< XPropertySet > xRet = pRet;
    return xRet;
}
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
Any SAL_CALL OColumns::queryInterface( const Type & rType ) throw(RuntimeException)
{
    if(!m_pTable || (m_pTable && !m_pTable->isNew()))
    {
        if(!m_bAddColumn    && rType == getCppuType( (Reference<XAppend>*)0))
            return Any();
        if(!m_bDropColumn   && rType == getCppuType( (Reference<XDrop>*)0))
            return Any();
    }

    return OColumns_BASE::queryInterface( rType);
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL OColumns::getTypes(  ) throw(RuntimeException)
{
    Type aAppendType = getCppuType( (Reference<XAppend>*)0);
    Type aDropType = getCppuType( (Reference<XDrop>*)0);

    Sequence< Type > aTypes(OColumns_BASE::getTypes());
    Sequence< Type > aRet(aTypes.getLength() -
        ((m_pTable && m_pTable->isNew()) ? 0 :
            ((m_bDropColumn ?
                (m_bAddColumn ? 0 : 1) : (m_bAddColumn ? 1 : 2)))));

    const Type* pBegin = aTypes.getConstArray();
    const Type* pEnd = pBegin + aTypes.getLength();
    for(sal_Int32 i=0;pBegin != pEnd ;++pBegin)
    {
        if(*pBegin != aAppendType && *pBegin != aDropType)
            aRet.getArray()[i++] = *pBegin;
        else if((m_pTable && m_pTable->isNew()) || (m_bDropColumn && *pBegin == aDropType))
            aRet.getArray()[i++] = *pBegin;
        else if((m_pTable && m_pTable->isNew()) || (m_bAddColumn && *pBegin == aAppendType))
            aRet.getArray()[i++] = *pBegin;
    }
    return aRet;
}
// -------------------------------------------------------------------------
// XAppend
void SAL_CALL OColumns::appendByDescriptor( const Reference< XPropertySet >& descriptor ) throw(SQLException, ElementExistException, RuntimeException)
{
    if(m_pTable && !m_pTable->isNew() && !m_bAddColumn)
        throw SQLException();

    ::osl::MutexGuard aGuard(m_rMutex);

    Reference<XAppend> xAppend(m_xDrvColumns,UNO_QUERY);
    if(xAppend.is())
    {
        xAppend->appendByDescriptor(descriptor);
    }
    else if(m_pTable && !m_pTable->isNew())
    {
        ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("ALTER TABLE ");
        ::rtl::OUString aQuote  = m_pTable->getMetaData()->getIdentifierQuoteString(  );
        ::rtl::OUString aDot    = ::rtl::OUString::createFromAscii(".");

        ::rtl::OUString aCatalog;
        ::rtl::OUString aSchema;
        ::rtl::OUString aTable;
        m_pTable->getPropertyValue(PROPERTY_CATALOGNAME)    >>= aCatalog;
        m_pTable->getPropertyValue(PROPERTY_SCHEMANAME)     >>= aSchema;
        m_pTable->getPropertyValue(PROPERTY_NAME)           >>= aTable;

        ::rtl::OUString aComposedName;
        dbtools::composeTableName(m_pTable->getMetaData(),aCatalog,aSchema,aTable,aComposedName,sal_True);

        aSql += aComposedName;
        aSql += ::rtl::OUString::createFromAscii(" ADD ");
        aSql += ::dbtools::quoteName( aQuote,::comphelper::getString(descriptor->getPropertyValue(PROPERTY_NAME)));
        aSql += ::rtl::OUString::createFromAscii(" ");

        sal_Int32 nType = connectivity::getINT32(descriptor->getPropertyValue(PROPERTY_TYPE));
        Any aTypeName = descriptor->getPropertyValue(PROPERTY_TYPENAME);
        if(aTypeName.hasValue() && connectivity::getString(aTypeName).getLength())
            aSql += connectivity::getString(aTypeName);
        else
        {
            sal_Int32 nPrec     = connectivity::getINT32(descriptor->getPropertyValue(PROPERTY_PRECISION));
            ::rtl::OUString aTypeName;
            //  sal_Int32 nScale    = getINT32(descriptor->getPropertyValue(PROPERTY_SCALE));
            Reference<XResultSet> xTypes(m_pTable->getMetaData()->getTypeInfo());
            Reference<XRow> xRow(xTypes,UNO_QUERY);
            while(xTypes->next())
            {
                aTypeName = xRow->getString(1); // must be fetched in order
                if(xRow->getInt(2) == nType && xRow->getInt(3) >= nPrec)
                    break;
            }

            aSql += aTypeName + ::rtl::OUString::createFromAscii(" ");
        }

        switch(nType)
        {
            case DataType::CHAR:
            case DataType::VARCHAR:
            case DataType::BINARY:
            case DataType::VARBINARY:
                aSql += ::rtl::OUString::createFromAscii("(")
                        + ::rtl::OUString::valueOf(connectivity::getINT32(descriptor->getPropertyValue(PROPERTY_PRECISION)))
                        + ::rtl::OUString::createFromAscii(")");
                break;

            case DataType::DECIMAL:
            case DataType::NUMERIC:
                aSql += ::rtl::OUString::createFromAscii("(")
                            + ::rtl::OUString::valueOf(connectivity::getINT32(descriptor->getPropertyValue(PROPERTY_PRECISION)))
                            + ::rtl::OUString::createFromAscii(",")
                            + ::rtl::OUString::valueOf(connectivity::getINT32(descriptor->getPropertyValue(PROPERTY_SCALE)))
                            + ::rtl::OUString::createFromAscii(")");
                break;
        }
        ::rtl::OUString aDefault = connectivity::getString(descriptor->getPropertyValue(PROPERTY_DEFAULTVALUE));

        if(aDefault.getLength())
            aSql += ::rtl::OUString::createFromAscii(" DEFAULT ") + aDefault;

        if(connectivity::getINT32(descriptor->getPropertyValue(PROPERTY_ISNULLABLE)) == ColumnValue::NO_NULLS)
            aSql += ::rtl::OUString::createFromAscii(" NOT NULL");

        Reference< XStatement > xStmt = m_pTable->getConnection()->createStatement(  );
        xStmt->execute(aSql);
    }

    OColumns_BASE::appendByDescriptor(descriptor);
}
// -------------------------------------------------------------------------
// XDrop
void SAL_CALL OColumns::dropByName( const ::rtl::OUString& elementName ) throw(SQLException, NoSuchElementException, RuntimeException)
{
    if(m_pTable && !m_pTable->isNew() && !m_bDropColumn)
        throw SQLException();

    ::osl::MutexGuard aGuard(m_rMutex);
    if(m_pTable && !m_pTable->isNew())
    {
        ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("ALTER TABLE ");
        ::rtl::OUString aQuote  = m_pTable->getMetaData()->getIdentifierQuoteString(  );
        ::rtl::OUString aDot    = ::rtl::OUString::createFromAscii(".");

        ::rtl::OUString aCatalog;
        ::rtl::OUString aSchema;
        ::rtl::OUString aTable;
        m_pTable->getPropertyValue(PROPERTY_CATALOGNAME)    >>= aCatalog;
        m_pTable->getPropertyValue(PROPERTY_SCHEMANAME)     >>= aSchema;
        m_pTable->getPropertyValue(PROPERTY_NAME)           >>= aTable;

        ::rtl::OUString aComposedName;
        dbtools::composeTableName(m_pTable->getMetaData(),aCatalog,aSchema,aTable,aComposedName,sal_True);

        aSql += aComposedName;
        aSql += ::rtl::OUString::createFromAscii(" DROP ");
        aSql += ::dbtools::quoteName( aQuote,elementName);

        Reference< XStatement > xStmt = m_pTable->getConnection()->createStatement(  );
        if(xStmt.is())
            xStmt->execute(aSql);
    }

    OColumns_BASE::dropByName(elementName);
}
// -------------------------------------------------------------------------
void SAL_CALL OColumns::dropByIndex( sal_Int32 index ) throw(SQLException, IndexOutOfBoundsException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    if (index < 0 || index >= getCount())
        throw IndexOutOfBoundsException();

    dropByName(m_aElements[index]->first);
}
/*

//============================================================
//= OColumn
//============================================================
DBG_NAME(OColumn);
//------------------------------------------------------------------------------
void OColumn::construct()
{
    m_nType = DataType::OTHER;
    m_nPrecision = 0;
    m_nNumberFormat = 0;
    m_nScale = 0;
    m_bAutoIncrement = m_bRowVersion = sal_False;
    m_nIsNullable = ColumnValue::NULLABLE_UNKNOWN;
    m_bHidden = sal_False;
}

//------------------------------------------------------------------------------
void OColumn::construct(const OColumn& _rSource)
{
    m_aAlignment = _rSource.m_aAlignment;
    m_aWidth = _rSource.m_aWidth;
    m_sName = _rSource.m_sName;
    m_aTypeName = _rSource.m_aTypeName;
    m_aDescription = _rSource.m_aDescription;
    m_aDefaultValue = _rSource.m_aDefaultValue;
    m_nType = _rSource.m_nType;
    m_nPrecision = _rSource.m_nPrecision;
    m_nScale = _rSource.m_nScale;
    m_nNumberFormat = _rSource.m_nNumberFormat;
    m_nIsNullable = _rSource.m_nIsNullable;
    m_bAutoIncrement = _rSource.m_bAutoIncrement;
    m_bRowVersion = _rSource.m_bRowVersion;
    m_bHidden = _rSource.m_bHidden;
}

//--------------------------------------------------------------------------
OColumn::OColumn()
        :OColumnBase(m_aMutex)
        ,OPropertySetHelper(OColumnBase::rBHelper)
{
    DBG_CTOR(OColumn, NULL);
    construct();
}

//--------------------------------------------------------------------------
OColumn::OColumn(const ::rtl::OUString& _rName)
        :OColumnBase(m_aMutex)
        ,OPropertySetHelper(OColumnBase::rBHelper)
        ,m_sName(_rName)
{
    DBG_CTOR(OColumn, NULL);
    construct();
}

//--------------------------------------------------------------------------
OColumn::OColumn(const OColumn& _rSource)
        :OColumnBase(m_aMutex)
        ,OPropertySetHelper(OColumnBase::rBHelper)
{
    DBG_CTOR(OColumn, NULL);
    construct(_rSource);
}

//--------------------------------------------------------------------------
OColumn::~OColumn()
{
    DBG_DTOR(OColumn, NULL);
}

// com::sun::star::lang::XTypeProvider
//--------------------------------------------------------------------------
Sequence< Type > OColumn::getTypes() throw (RuntimeException)
{
    OTypeCollection aTypes(::getCppuType( (const Reference< XPropertySet > *)0 ),
                            OColumnBase::getTypes() );

    return aTypes.getTypes();
}

//--------------------------------------------------------------------------
Sequence< sal_Int8 > OColumn::getImplementationId() throw (RuntimeException)
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

// com::sun::star::uno::XInterface
//--------------------------------------------------------------------------
Any OColumn::queryInterface( const Type & rType ) throw (RuntimeException)
{
    Any aIface = OColumnBase::queryInterface( rType );
    if (!aIface.hasValue())
        aIface = ::cppu::queryInterface(
                    rType,
                    static_cast< XPropertySet * >( this ));
    return aIface;
}

//--------------------------------------------------------------------------
void OColumn::acquire() throw(RuntimeException)
{
    OColumnBase::acquire();
}

//--------------------------------------------------------------------------
void OColumn::release() throw(RuntimeException)
{
    OColumnBase::release();
}

// OComponentHelper
//------------------------------------------------------------------------------
void OColumn::disposing()
{
    OPropertySetHelper::disposing();
}

// OPropertySetHelper
//------------------------------------------------------------------------------
void OColumn::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_ALIGN:
            rValue = m_aAlignment;
            break;
        case PROPERTY_ID_HIDDEN:
        {
            sal_Bool bVal = m_bHidden;
            rValue.setValue(&bVal, getBooleanCppuType());
        }
        case PROPERTY_ID_WIDTH:
            rValue = m_aWidth;
            break;
        case PROPERTY_ID_TYPE:
            rValue <<= m_nType;
            break;
        case PROPERTY_ID_PRECISION:
            rValue <<= m_nPrecision;
            break;
        case PROPERTY_ID_SCALE:
            rValue <<= m_nScale;
            break;
        case PROPERTY_ID_ISNULLABLE:
            rValue <<= m_nIsNullable;
            break;
        case PROPERTY_ID_NUMBERFORMAT:
            rValue <<= m_nNumberFormat;
            break;
        case PROPERTY_ID_TYPENAME:
            rValue <<= m_aTypeName;
            break;
        case PROPERTY_ID_DESCRIPTION:
            rValue <<= m_aDescription;
            break;
        case PROPERTY_ID_DEFAULTVALUE:
            rValue <<= m_aDefaultValue;
            break;
        case PROPERTY_ID_ISAUTOINCREMENT:
        {
            sal_Bool bVal = m_bAutoIncrement;
            rValue.setValue(&bVal, getBooleanCppuType());
        }   break;
        case PROPERTY_ID_ISROWVERSION:
        {
            sal_Bool bVal = m_bRowVersion;
            rValue.setValue(&bVal, getBooleanCppuType());
        }   break;
        case PROPERTY_ID_NAME:
            rValue <<= m_sName;
            break;
        default:
            DBG_ERROR("unknown Property");
    }
}

//------------------------------------------------------------------------------
sal_Bool OColumn::convertFastPropertyValue(
                            Any & rConvertedValue,
                            Any & rOldValue,
                            sal_Int32 nHandle,
                            const Any& rValue )
                                throw (IllegalArgumentException)
{
    sal_Bool bModified = sal_False;
    switch (nHandle)
    {
        case PROPERTY_ID_ALIGN:
            bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aAlignment,
                ::getCppuType(static_cast< sal_Int32* >(NULL)));
            break;
        case PROPERTY_ID_WIDTH:
            bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aWidth,
                ::getCppuType(static_cast< sal_Int32* >(NULL)));
            break;
        case PROPERTY_ID_HIDDEN:
            bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_bHidden);
            break;
    }
    return bModified;
}

//------------------------------------------------------------------------------
void OColumn::setFastPropertyValue_NoBroadcast(
                                sal_Int32 nHandle,
                                const Any& rValue
                                                 )
                                                 throw (Exception)
{
    switch (nHandle)
    {
        case PROPERTY_ID_ALIGN:
            OSL_ENSURE(!rValue.hasValue() || rValue.getValueType().equals(::getCppuType(static_cast< sal_Int32* >(NULL))),
                "OColumn::setFastPropertyValue_NoBroadcast(ALIGN) : invalid value !");
            m_aAlignment = rValue;
            break;
        case PROPERTY_ID_WIDTH:
            OSL_ENSURE(!rValue.hasValue() || rValue.getValueType().equals(::getCppuType(static_cast< sal_Int32* >(NULL))),
                "OColumn::setFastPropertyValue_NoBroadcast(WIDTH) : invalid value !");
            m_aWidth = rValue;
            break;
        case PROPERTY_ID_HIDDEN:
            OSL_ENSURE(rValue.getValueType().equals(::getBooleanCppuType()),
                "OColumn::setFastPropertyValue_NoBroadcast(HIDDEN) : invalid value !");
            m_bHidden = ::comphelper::getBOOL(rValue);
            break;
    }
}

//------------------------------------------------------------------------------
Reference< XPropertySetInfo > OColumn::getPropertySetInfo() throw (RuntimeException)
{
    return createPropertySetInfo( getInfoHelper() ) ;
}

// comphelper::OPropertyArrayUsageHelper
//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OColumn::createArrayHelper( ) const
{
    BEGIN_PROPERTY_HELPER(14)
        DECL_PROP2(ALIGN,               sal_Int32,          BOUND, MAYBEVOID);
        DECL_PROP1(DEFAULTVALUE,        ::rtl::OUString,    READONLY);
        DECL_PROP1(DESCRIPTION,         ::rtl::OUString,    READONLY);
        DECL_PROP1(NUMBERFORMAT,        sal_Int32,          BOUND);
        DECL_BOOL_PROP1(ISAUTOINCREMENT,                    READONLY);
        DECL_BOOL_PROP1(HIDDEN,                             BOUND);
        DECL_PROP1(ISNULLABLE,          sal_Int32,          READONLY);
        DECL_BOOL_PROP1(ISROWVERSION,                       READONLY);
        DECL_PROP1(NAME,                ::rtl::OUString,    READONLY);
        DECL_PROP1(PRECISION,           sal_Int32,          READONLY);
        DECL_PROP1(SCALE,               sal_Int32,          READONLY);
        DECL_PROP1(TYPE,                sal_Int32,          READONLY);
        DECL_PROP1(TYPENAME,            ::rtl::OUString,    READONLY);
        DECL_PROP2(WIDTH,               sal_Int32,          BOUND, MAYBEVOID);
    END_PROPERTY_HELPER();
}

// cppu::OPropertySetHelper
//------------------------------------------------------------------------------
IPropertyArrayHelper& OColumn::getInfoHelper()
{
    return *getArrayHelper();
}

// XNamed
//------------------------------------------------------------------------------
::rtl::OUString OColumn::getName( ) throw(RuntimeException)
{
    return m_sName;
}

//------------------------------------------------------------------------------
void OColumn::setName( const ::rtl::OUString& aName ) throw(RuntimeException)
{
    // name setting not allowed
}

// XServiceInfo
//------------------------------------------------------------------------------
rtl::OUString OColumn::getImplementationName(  ) throw(RuntimeException)
{
    return rtl::OUString::createFromAscii("com.sun.star.sdb.OColumn");
}

//------------------------------------------------------------------------------
sal_Bool OColumn::supportsService( const ::rtl::OUString& _rServiceName ) throw (RuntimeException)
{
    return ::comphelper::findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > OColumn::getSupportedServiceNames(  ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aSNS( 1 );
    aSNS[0] = SERVICE_SDB_COLUMN;
    return aSNS;
}

//============================================================
//= OColumns
//============================================================
DBG_NAME(OColumns);

//--------------------------------------------------------------------------
OColumns::OColumns(::cppu::OWeakObject& _rParent, ::osl::Mutex& _rMutex, sal_Bool _bCaseSensitive)
    :m_rParent(_rParent)
    ,m_rMutex(_rMutex)
    ,m_bCaseSensitive(_bCaseSensitive)
{
    m_pColMap = new OColumnMap(17, ::comphelper::UStringMixHash(_bCaseSensitive), ::comphelper::UStringMixEqual(_bCaseSensitive));
}

//--------------------------------------------------------------------------
OColumns::~OColumns()
{
    DELETEZ(m_pColMap);
}

// XServiceInfo
//------------------------------------------------------------------------------
rtl::OUString OColumns::getImplementationName(  ) throw(RuntimeException)
{
    return rtl::OUString::createFromAscii("com.sun.star.sdb.OColumns");
}

//------------------------------------------------------------------------------
sal_Bool OColumns::supportsService( const ::rtl::OUString& _rServiceName ) throw (RuntimeException)
{
    return ::comphelper::findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > OColumns::getSupportedServiceNames(  ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aSNS( 2 );
    aSNS[0] = SERVICE_SDBCX_CONTAINER;
    aSNS[1] = SERVICE_SDBCX_COLUMNS;
    return aSNS;
}

// XElementAccess
//------------------------------------------------------------------------------
Type SAL_CALL OColumns::getElementType(  ) throw(RuntimeException)
{
    return::getCppuType(static_cast<Reference<XPropertySet>*>(NULL));
}

//------------------------------------------------------------------------------
sal_Bool OColumns::hasElements(void) throw( RuntimeException )
{
    MutexGuard aGuard(m_rMutex);
    return !m_aColArray.empty();
}

// XEnumerationAccess
//------------------------------------------------------------------------------
Reference< XEnumeration >  OColumns::createEnumeration(void) throw( RuntimeException )
{
    MutexGuard aGuard(m_rMutex);
    return new ::comphelper::OEnumerationByIndex(this);
}

// XIndexAccess
//------------------------------------------------------------------------------
sal_Int32 OColumns::getCount( ) throw( RuntimeException )
{
    MutexGuard aGuard(m_rMutex);
    return m_aColArray.size();
}

//------------------------------------------------------------------------------
Any OColumns::getByIndex( sal_Int32 Index ) throw(IndexOutOfBoundsException,
                                                  WrappedTargetException,
                                                  RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    if (Index < 0 || Index >= m_aColArray.size())
        throw IndexOutOfBoundsException();

    return makeAny(Reference< XPropertySet >(m_aColArray[Index]));
}

// XNameAccess
//------------------------------------------------------------------------------
Any OColumns::getByName(const ::rtl::OUString& aName) throw(NoSuchElementException,
                                                            WrappedTargetException,
                                                            RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    OColumnMap::const_iterator i = m_pColMap->find(aName);
    if (i == m_pColMap->end())
        throw NoSuchElementException();

    return makeAny(Reference< XPropertySet >((*i).second));
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > OColumns::getElementNames(void) throw(RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    Sequence< ::rtl::OUString > aNameList(m_aColArray.size());
    ::rtl::OUString* pStringArray = aNameList.getArray();
    for (OColumnArray::const_iterator i = m_aColArray.begin(); m_aColArray.end() != i; i++)
    {
        (*pStringArray) = (*i)->m_sName;
        ++pStringArray;
    }
    return aNameList;
}

//------------------------------------------------------------------------------
void SAL_CALL OColumns::appendByDescriptor( const Reference< XPropertySet >& descriptor ) throw(SQLException, ElementExistException, RuntimeException)
{
    // TODO
}

//------------------------------------------------------------------------------
void SAL_CALL OColumns::dropByName( const ::rtl::OUString& elementName ) throw(SQLException, NoSuchElementException, RuntimeException)
{
    // TODO
}

//------------------------------------------------------------------------------
void SAL_CALL OColumns::dropByIndex( sal_Int32 index ) throw(SQLException, IndexOutOfBoundsException, RuntimeException)
{
    // TODO
}

//------------------------------------------------------------------------------
sal_Bool OColumns::hasByName( const ::rtl::OUString& aName ) throw(RuntimeException)
{
    MutexGuard aGuard(m_rMutex);
    OColumnMap::const_iterator i = m_pColMap->find(aName);
    return i != m_pColMap->end();
}

//------------------------------------------------------------------------------
sal_Int32 OColumns::findColumn(const rtl::OUString& columnName) throw( SQLException, RuntimeException )
{
    MutexGuard aGuard(m_rMutex);
    OColumnMap::const_iterator i = m_pColMap->find(columnName);
    if (i == m_pColMap->end())
        return 0;

    return find(m_aColArray.begin(), m_aColArray.end(), (*i).second) -  m_aColArray.begin() + 1;
}

//------------------------------------------------------------------------------
void OColumns::disposing()
{
    MutexGuard aGuard(m_rMutex);
    for (OColumnArray::iterator i = m_aColArray.begin(); m_aColArray.end() != i; i++)
    {
        (*i)->dispose();
        (*i)->release();
    }
    m_aColArray.clear();
    m_pColMap->clear();
}

#if 0
//------------------------------------------------------------------
const OColumn* OColumns::at(const rtl::OUString& rName) const
{
    OColumnMap::const_iterator i = m_pColMap->find(rName);
    return i == m_pColMap->end() ? NULL : (*i).second;
}

#endif

//------------------------------------------------------------------
void OColumns::setCaseSensitive(sal_Bool bCaseSensitive)
{
    MutexGuard aGuard(m_rMutex);
    OSL_ENSURE(m_pColMap->empty(), "OColumns::setCaseSensitive() there are still existing columns");
    if (bCaseSensitive == m_pColMap->hash_funct().isCaseSensitive())
        return;

    delete(m_pColMap);
    m_pColMap = new OColumnMap(17, ::comphelper::UStringMixHash(bCaseSensitive), ::comphelper::UStringMixEqual(bCaseSensitive));
}

//------------------------------------------------------------------
void OColumns::append(OColumn* pCol)
{
    MutexGuard aGuard(m_rMutex);
    pCol->acquire();
    (*m_pColMap)[pCol->m_sName] = pCol;
    m_aColArray.push_back(pCol);
}

//------------------------------------------------------------------
void OColumns::clearColumns()
{
    MutexGuard aGuard(m_rMutex);
    m_aColArray.clear();
    m_pColMap->clear();
}
*/

