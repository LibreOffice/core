/*************************************************************************
 *
 *  $RCSfile: column.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:15:38 $
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
#ifndef _DBASHARED_STRINGCONSTANTS_HRC_
#include "stringconstants.hrc"
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef _DBA_CORE_SEQUENCEOUTPUTSTREAM_HXX_
#include "seqoutputstream.hxx"
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
#ifndef _UTL_SEQUENCE_HXX_
#include <unotools/sequence.hxx>
#endif
#ifndef _UTL_PROPERTY_HXX_
#include <unotools/property.hxx>
#endif
#ifndef _UNOTOOLS_ENUMHELPER_HXX_
#include <unotools/enumhelper.hxx>
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _UTL_SEQSTREAM_HXX
#include <unotools/seqstream.hxx>
#endif
#ifndef _UTL_BASIC_IO_HXX_
#include <unotools/basicio.hxx>
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
using namespace ::utl;
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
    return ::utl::findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
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
            bModified = ::utl::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_sName);
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
            OSL_ENSHURE(rValue.getValueType().equals(::getCppuType(static_cast< ::rtl::OUString* >(NULL))),
                "OColumn::setFastPropertyValue_NoBroadcast(ALIGN) : invalid value !");
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
            bModified = ::utl::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aAlignment,
                ::getCppuType(static_cast< sal_Int32* >(NULL)));
            break;
        case PROPERTY_ID_WIDTH:
            bModified = ::utl::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aWidth,
                ::getCppuType(static_cast< sal_Int32* >(NULL)));
            break;
        case PROPERTY_ID_HIDDEN:
            bModified = ::utl::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_bHidden);
            break;
        case PROPERTY_ID_RELATIVEPOSITION:
            bModified = ::utl::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aRelativePosition,
                ::getCppuType(static_cast< sal_Int32* >(NULL)));
            break;
        case PROPERTY_ID_NUMBERFORMAT:
            bModified = ::utl::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aFormatKey,
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
            OSL_ENSHURE(!rValue.hasValue() || rValue.getValueType().equals(::getCppuType(static_cast< sal_Int32* >(NULL))),
                "OColumnSettings::setFastPropertyValue_NoBroadcast(ALIGN) : invalid value !");
            m_aAlignment = rValue;
            break;
        case PROPERTY_ID_WIDTH:
            OSL_ENSHURE(!rValue.hasValue() || rValue.getValueType().equals(::getCppuType(static_cast< sal_Int32* >(NULL))),
                "OColumnSettings::setFastPropertyValue_NoBroadcast(WIDTH) : invalid value !");
            m_aWidth = rValue;
            break;
        case PROPERTY_ID_NUMBERFORMAT:
            OSL_ENSHURE(!rValue.hasValue() || rValue.getValueType().equals(::getCppuType(static_cast< sal_Int32* >(NULL))),
                "OColumnSettings::setFastPropertyValue_NoBroadcast(NUMBERFORMAT) : invalid value !");
            m_aWidth = rValue;
            break;
        case PROPERTY_ID_RELATIVEPOSITION:
            OSL_ENSHURE(!rValue.hasValue() || rValue.getValueType().equals(::getCppuType(static_cast< sal_Int32* >(NULL))),
                "OColumnSettings::setFastPropertyValue_NoBroadcast(ID_RELATIVEPOSITION) : invalid value !");
            m_aWidth = rValue;
            break;
        case PROPERTY_ID_HIDDEN:
            OSL_ENSHURE(rValue.getValueType().equals(::getBooleanCppuType()),
                "OColumnSettings::setFastPropertyValue_NoBroadcast(HIDDEN) : invalid value !");
            m_bHidden = ::utl::getBOOL(rValue);
            break;
    }
}


//------------------------------------------------------------------------------
sal_Bool OColumnSettings::writeUITo(const Reference< XRegistryKey >& _rxConfigNode)
{
    sal_Bool bWroteAnything = sal_False;
    sal_Int32 nTemp;

    if (m_aAlignment.hasValue())
    {
        m_aAlignment >>= nTemp;
        writeValue(_rxConfigNode, CONFIGKEY_COLUMN_ALIGNMENT, nTemp);
        bWroteAnything = sal_True;
    }
    else
        deleteKey(_rxConfigNode, CONFIGKEY_COLUMN_ALIGNMENT);

    if (m_aWidth.hasValue())
    {
        m_aWidth >>= nTemp;
        writeValue(_rxConfigNode, CONFIGKEY_COLUMN_WIDTH, nTemp);
        bWroteAnything = sal_True;
    }
    else
        deleteKey(_rxConfigNode, CONFIGKEY_COLUMN_WIDTH);

    if (m_aFormatKey.hasValue())
    {
        m_aFormatKey >>= nTemp;
        writeValue(_rxConfigNode, CONFIGKEY_COLUMN_NUMBERFORMAT, nTemp);
        bWroteAnything = sal_True;
    }
    else
        deleteKey(_rxConfigNode, CONFIGKEY_COLUMN_NUMBERFORMAT);

    if (m_aRelativePosition.hasValue())
    {
        m_aRelativePosition >>= nTemp;
        writeValue(_rxConfigNode, CONFIGKEY_COLUMN_RELPOSITION, nTemp);
        bWroteAnything = sal_True;
    }
    else
        deleteKey(_rxConfigNode, CONFIGKEY_COLUMN_RELPOSITION);

    if (m_bHidden)
    {
        writeValue(_rxConfigNode, CONFIGKEY_COLUMN_HIDDEN, m_bHidden);
        bWroteAnything = sal_True;
    }
    else
        deleteKey(_rxConfigNode, CONFIGKEY_COLUMN_HIDDEN);

    return bWroteAnything;
}

//------------------------------------------------------------------------------
void OColumnSettings::readUIFrom(const Reference< XRegistryKey >& _rxConfigNode)
{
    // some defaults
    m_bHidden = sal_False;
    m_aRelativePosition.clear();
    m_aFormatKey.clear();
    m_aWidth.clear();
    m_aAlignment.clear();

    sal_Int32 nTemp(0);
    if (readValue(_rxConfigNode, CONFIGKEY_COLUMN_ALIGNMENT, nTemp))
        m_aAlignment <<= nTemp;
    if (readValue(_rxConfigNode, CONFIGKEY_COLUMN_WIDTH, nTemp))
        m_aWidth <<= nTemp;
    if (readValue(_rxConfigNode, CONFIGKEY_COLUMN_NUMBERFORMAT, nTemp))
        m_aFormatKey <<= nTemp;
    if (readValue(_rxConfigNode, CONFIGKEY_COLUMN_RELPOSITION, nTemp))
        m_aRelativePosition <<= nTemp;
    sal_Bool bTemp(sal_False);
    if (readValue(_rxConfigNode, CONFIGKEY_COLUMN_HIDDEN, bTemp))
        m_bHidden = bTemp;
}

//============================================================
//= OColumns
//============================================================
DBG_NAME(OColumns);

//--------------------------------------------------------------------------
OColumns::OColumns(::cppu::OWeakObject& _rParent,
                   ::osl::Mutex& _rMutex,
                   sal_Bool _bCaseSensitive,const ::std::vector< ::rtl::OUString> &_rVector,
                   sal_Bool _bAddColumn,sal_Bool _bDropColumn)
                   : connectivity::sdbcx::OCollection(_rParent,_bCaseSensitive,_rMutex,_rVector)
                   ,m_pTable(NULL)
//  :m_rParent(_rParent)
//  ,m_rMutex(_rMutex)
    ,m_bInitialized(sal_False)
    ,m_bAddColumn(_bAddColumn)
    ,m_bDropColumn(_bDropColumn)
{
    //  m_pColMap = new OColumnMap(17, ::utl::UStringMixHash(_bCaseSensitive), ::utl::UStringMixEqual(_bCaseSensitive));
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
    return ::utl::findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
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
    //  m_aColArray.clear();
    //  m_pColMap->clear();
    m_aNameMap.clear();
    m_aElements.clear();
}

//------------------------------------------------------------------------------
void OColumns::loadSettings(const Reference< XRegistryKey >& _rxLocation, const IColumnFactory* _pColFactory, sal_Bool _bAdjustLocation)
{
    MutexGuard aGuard(m_rMutex);

    OSL_ENSHURE(_pColFactory != NULL, "OColumns::loadSettings : invalid factory !");

    ORegistryLevelEnumeration aSubKeys(_rxLocation);
    while (aSubKeys.hasMoreElements())
    {
        Reference< XRegistryKey > xCurrent = aSubKeys.nextElement();

        ::rtl::OUString sColName;
        if (!readValue(xCurrent, CONFIGKEY_CONTAINERLEMENT_TITLE, sColName) || !sColName.getLength())
            // no title key or invalid title
            continue;

        Reference< XRegistryKey > xObjectKey;
        if (!openKey(xCurrent, CONFIGKEY_CONTAINERLEMENT_OBJECT, xObjectKey, sal_False))
            // there is no valid object key
            continue;

        // do we already have a column with that name ?
        //  OColumnMap::iterator aExistent = m_pColMap->find(sColName);
        //  OColumn* pExistent = aExistent != m_pColMap->end() ? aExistent->second : NULL;

        OColumn* pExistent = NULL;
        // create the column if neccessary
        if (!hasByName(sColName))
        {
            pExistent = _pColFactory->createColumn(sColName);
            if (pExistent)
                append(sColName, pExistent);
            else
            {
                OSL_ASSERT("OColumns::loadSettings : createColumn returned nonsense !");
                continue;
            }
        }
        else
        {
            Reference<XNamed> xColumn;
            getByName(sColName) >>= xColumn;
            Reference< ::com::sun::star::lang::XUnoTunnel> xTunnel(xColumn,UNO_QUERY);
            if(xTunnel.is())
                pExistent = (OColumn*)xTunnel->getSomething(OColumn::getUnoTunnelImplementationId());
            OSL_ENSHURE(pExistent,"OColumns::loadSettings: No column from unotunnelhelper!");
        }

        OColumnSettings* pExistentSettings = pExistent->getSettings();
        if (pExistentSettings)
            pExistentSettings->readUIFrom(xObjectKey);
        else
            OSL_ASSERT("OColumns::loadSettings : no settings for the column !");
    }

    // adjust the configuration node
    if (_bAdjustLocation)
        m_xConfigurationNode = _rxLocation;
}

//------------------------------------------------------------------------------
void OColumns::storeSettings()
{
    MutexGuard aGuard(m_rMutex);
    if (!m_xConfigurationNode.is())
    {
        OSL_ASSERT("OColumns::storeSettings : have no location !");
        return;
    }
    if (m_xConfigurationNode->isReadOnly())
    {
        OSL_ASSERT("OColumns::storeSettings : the location is read-only !");
        return;
    }

    DECLARE_STL_USTRINGACCESS_MAP(Reference< XRegistryKey >, MapName2Node);
    MapName2Node aDescKeys;
    MapName2Node aObjectKeys;

    // collect the sub keys of existent column descriptions
    ORegistryLevelEnumeration aSubKeys(m_xConfigurationNode);
    while (aSubKeys.hasMoreElements())
    {
        Reference< XRegistryKey > xCurrent = aSubKeys.nextElement();
        ::rtl::OUString sColName;
        if (!readValue(xCurrent, CONFIGKEY_CONTAINERLEMENT_TITLE, sColName) || !sColName.getLength())
        {   // no title key or invalid title -> delete that sub key
            deleteKey(m_xConfigurationNode, getShortKeyName(xCurrent));
            continue;
        }

        Reference< XRegistryKey > xObjectKey;
        if (!openKey(xCurrent, CONFIGKEY_CONTAINERLEMENT_OBJECT, xObjectKey, sal_False))
        {   // no object key -> delete and continue
            deleteKey(m_xConfigurationNode, getShortKeyName(xCurrent));
            continue;
        }

        aDescKeys[sColName] = xCurrent;
        aObjectKeys[sColName] = xObjectKey;
    }

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
            pCurrent = (OColumn*)xTunnel->getSomething(OColumn::getUnoTunnelImplementationId());

        OSL_ENSHURE(pCurrent,"OColumns::storeSettings: No column from unotunnelhelper!");

        OColumnSettings* pCurrentSettings = pCurrent->getSettings();
        if (!pCurrentSettings)
        {
            OSL_ASSERT("OColumns::storeSettings : column without settings we can write !");
            continue;
        }
        sCurrent = pCurrent->m_sName;

        Reference< XRegistryKey > xColumnNode;
        Reference< XRegistryKey > xDescKey;
        // do we we have an existent key for that column ?
        ConstMapName2NodeIterator aExistentObjectKey = aObjectKeys.find(sCurrent);
        if (aExistentObjectKey != aObjectKeys.end())
        {
            xColumnNode = aExistentObjectKey->second;

            ConstMapName2NodeIterator aExistentDescKey = aDescKeys.find(sCurrent);
            OSL_ENSHURE(aExistentDescKey != aDescKeys.end(), "OColumns::storeSettings : inconsistent : no description key !");
            xDescKey = aExistentDescKey->second;
            // and so these sub key is used (and must not be deleted afterwards)
            // -> remove from the key maps
            aObjectKeys.erase(sCurrent);
            aDescKeys.erase(sCurrent);
        }
        else
        {   // no -> create one
            ::rtl::OUString sNewDescKey = getUniqueKeyName(m_xConfigurationNode, ::rtl::OUString::createFromAscii("col_"));
            if  (   !openKey(m_xConfigurationNode, sNewDescKey, xDescKey, sal_True)
                ||  !writeValue(xDescKey, CONFIGKEY_CONTAINERLEMENT_TITLE, sCurrent)
                ||  !openKey(xDescKey, CONFIGKEY_CONTAINERLEMENT_OBJECT, xColumnNode, sal_True)
                )

            {
                deleteKey(m_xConfigurationNode, getShortKeyName(xDescKey));
                OSL_ASSERT("OColumns::storeSettings : could not create the structures for writing a column !");
                continue;
            }
        }

        // let the column write itself
        if (!pCurrentSettings->writeUITo(xColumnNode))
        {   // the column did have only default values, so nothing was written
            aObjectKeys[sCurrent] = xColumnNode;
            aDescKeys[sCurrent] = xDescKey;
        }
    }

    // delete all description keys where we have no columns for
    for (   ConstMapName2NodeIterator   aRemove = aDescKeys.begin();
            aRemove != aDescKeys.end();
            ++aRemove
        )
    {
        deleteKey(m_xConfigurationNode, getShortKeyName(aRemove->second));
    }
}

//------------------------------------------------------------------------------
void OColumns::storeSettingsTo(const Reference< XRegistryKey >& _rxLocation)
{
    MutexGuard aGuard(m_rMutex);

    // temporary set our member to the new location
    Reference< XRegistryKey > xOldLocation(m_xConfigurationNode);
    m_xConfigurationNode = _rxLocation;

    try
    {
        storeSettings();
    }
    catch (...)
    {
        m_xConfigurationNode = xOldLocation;
        throw;
    }
    m_xConfigurationNode = xOldLocation;
}

//------------------------------------------------------------------------------
void OColumns::storeSettingsAs(const Reference< XRegistryKey >& _rxLocation)
{
    MutexGuard aGuard(m_rMutex);

    storeSettingsTo(_rxLocation);
    // success -> we have a new location
    m_xConfigurationNode = _rxLocation;
}
// -------------------------------------------------------------------------
void OColumns::impl_refresh() throw(::com::sun::star::uno::RuntimeException)
{
}
// -------------------------------------------------------------------------
Reference< XNamed > OColumns::createObject(const ::rtl::OUString& _rName)
{
    OSL_ENSHURE(m_pTable,"OColumns::createObject: ParentTable wasn't set");
    if(!m_pTable)
        return Reference< XNamed >();

    ::rtl::OUString aSchema,aTable;
    m_pTable->getPropertyValue(PROPERTY_SCHEMANAME) >>= aSchema;
    m_pTable->getPropertyValue(PROPERTY_NAME)       >>= aTable;

    Reference< XResultSet > xResult = m_pTable->getConnection()->getMetaData()->getColumns(m_pTable->getPropertyValue(PROPERTY_CATALOGNAME),
            aSchema,aTable,_rName);

    Reference< XNamed > xRet = NULL;
    if(xResult.is())
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        while(xResult->next())
        {
            if(xRow->getString(4) == _rName)
            {
                sal_Int32       nField5 = xRow->getInt(5);
                ::rtl::OUString aField6 = xRow->getString(6);
                sal_Int32       nField7 = xRow->getInt(7)
                            ,   nField9 = xRow->getInt(9)
                            ,   nField11= xRow->getInt(11);


                connectivity::sdbcx::OColumn* pRet = new connectivity::sdbcx::OColumn(_rName,
                                            aField6,
                                            xRow->getString(13),
                                            nField11,
                                            nField7,
                                            nField9,
                                            nField5,
                                            sal_False,sal_False,sal_False,sal_True);
                xRet = pRet;
                break;
            }
        }
    }

    return xRet;
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OColumns::createEmptyObject()
{
    OSL_ASSERT("Are not filled this way!");
    connectivity::sdbcx::OColumn* pRet = new connectivity::sdbcx::OColumn(isCaseSensitive());
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
    if(m_pTable && !m_pTable->isNew())
    {
        ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("ALTER TABLE ");
        ::rtl::OUString aQuote  = m_pTable->getConnection()->getMetaData()->getIdentifierQuoteString(  );
        ::rtl::OUString aDot    = ::rtl::OUString::createFromAscii(".");

        ::rtl::OUString aCatalog;
        ::rtl::OUString aSchema;
        ::rtl::OUString aTable;
        m_pTable->getPropertyValue(PROPERTY_CATALOGNAME)    >>= aCatalog;
        m_pTable->getPropertyValue(PROPERTY_SCHEMANAME)     >>= aSchema;
        m_pTable->getPropertyValue(PROPERTY_NAME)           >>= aTable;

        ::rtl::OUString aComposedName;
        composeTableName(m_pTable->getConnection()->getMetaData(),aCatalog,aSchema,aTable,aComposedName,sal_True);

        aSql += aComposedName;
        aSql += ::rtl::OUString::createFromAscii(" ADD ");
        aSql += aQuote + connectivity::getString(descriptor->getPropertyValue(PROPERTY_NAME)) + aQuote;
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
            Reference<XResultSet> xTypes(m_pTable->getConnection()->getMetaData()->getTypeInfo());
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
        ::rtl::OUString aQuote  = m_pTable->getConnection()->getMetaData()->getIdentifierQuoteString(  );
        ::rtl::OUString aDot    = ::rtl::OUString::createFromAscii(".");

        ::rtl::OUString aCatalog;
        ::rtl::OUString aSchema;
        ::rtl::OUString aTable;
        m_pTable->getPropertyValue(PROPERTY_CATALOGNAME)    >>= aCatalog;
        m_pTable->getPropertyValue(PROPERTY_SCHEMANAME)     >>= aSchema;
        m_pTable->getPropertyValue(PROPERTY_NAME)           >>= aTable;

        ::rtl::OUString aComposedName;
        composeTableName(m_pTable->getConnection()->getMetaData(),aCatalog,aSchema,aTable,aComposedName,sal_True);

        aSql += aComposedName;
        aSql += ::rtl::OUString::createFromAscii(" DROP ");
        aSql += aQuote + elementName + aQuote;

        Reference< XStatement > xStmt = m_pTable->getConnection()->createStatement(  );
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
            bModified = ::utl::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aAlignment,
                ::getCppuType(static_cast< sal_Int32* >(NULL)));
            break;
        case PROPERTY_ID_WIDTH:
            bModified = ::utl::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aWidth,
                ::getCppuType(static_cast< sal_Int32* >(NULL)));
            break;
        case PROPERTY_ID_HIDDEN:
            bModified = ::utl::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_bHidden);
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
            OSL_ENSHURE(!rValue.hasValue() || rValue.getValueType().equals(::getCppuType(static_cast< sal_Int32* >(NULL))),
                "OColumn::setFastPropertyValue_NoBroadcast(ALIGN) : invalid value !");
            m_aAlignment = rValue;
            break;
        case PROPERTY_ID_WIDTH:
            OSL_ENSHURE(!rValue.hasValue() || rValue.getValueType().equals(::getCppuType(static_cast< sal_Int32* >(NULL))),
                "OColumn::setFastPropertyValue_NoBroadcast(WIDTH) : invalid value !");
            m_aWidth = rValue;
            break;
        case PROPERTY_ID_HIDDEN:
            OSL_ENSHURE(rValue.getValueType().equals(::getBooleanCppuType()),
                "OColumn::setFastPropertyValue_NoBroadcast(HIDDEN) : invalid value !");
            m_bHidden = ::utl::getBOOL(rValue);
            break;
    }
}

//------------------------------------------------------------------------------
Reference< XPropertySetInfo > OColumn::getPropertySetInfo() throw (RuntimeException)
{
    return createPropertySetInfo( getInfoHelper() ) ;
}

// utl::OPropertyArrayUsageHelper
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
    return ::utl::findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
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
    m_pColMap = new OColumnMap(17, ::utl::UStringMixHash(_bCaseSensitive), ::utl::UStringMixEqual(_bCaseSensitive));
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
    return ::utl::findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
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
    return new ::utl::OEnumerationByIndex(this);
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
    OSL_ENSHURE(m_pColMap->empty(), "OColumns::setCaseSensitive() there are still existing columns");
    if (bCaseSensitive == m_pColMap->hash_funct().isCaseSensitive())
        return;

    delete(m_pColMap);
    m_pColMap = new OColumnMap(17, ::utl::UStringMixHash(bCaseSensitive), ::utl::UStringMixEqual(bCaseSensitive));
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

