/*************************************************************************
 *
 *  $RCSfile: definitioncolumn.cxx,v $
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

#ifndef _DBA_COREAPI_DEFINITIONSETTINGS_HXX_
#include "definitioncolumn.hxx"
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _UTL_PROPERTY_HXX_
#include <unotools/property.hxx>
#endif
#ifndef _DBASHARED_STRINGCONSTANTS_HRC_
#include "stringconstants.hrc"
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::cppu;
using namespace ::osl;
using namespace dbaccess;

#define HAS_DESCRIPTION     0x00000001
#define HAS_DEFAULTVALUE    0x00000002
#define HAS_ROWVERSION      0x00000004

//============================================================
//= OTableColumnDescriptor
//============================================================
// com::sun::star::lang::XTypeProvider
//--------------------------------------------------------------------------
Sequence< sal_Int8 > OTableColumnDescriptor::getImplementationId() throw (RuntimeException)
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

// ::com::sun::star::lang::XServiceInfo
//------------------------------------------------------------------------------
rtl::OUString OTableColumnDescriptor::getImplementationName(  ) throw (RuntimeException)
{
    return rtl::OUString::createFromAscii("com.sun.star.sdb.OTableColumnDescriptor");
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > OTableColumnDescriptor::getSupportedServiceNames(  ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aSNS( 2 );
    aSNS[0] = SERVICE_SDBCX_COLUMNDESCRIPTOR;
    aSNS[1] = SERVICE_SDB_COLUMNSETTINGS;
    return aSNS;
}

// utl::OPropertyArrayUsageHelper
//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OTableColumnDescriptor::createArrayHelper( ) const
{
    BEGIN_PROPERTY_HELPER(17)
        DECL_PROP1(ALIGN,               sal_Int32,          MAYBEVOID);
        DECL_PROP0_IFACE(CONTROLMODEL,  XPropertySet        );
        DECL_PROP0(DEFAULTVALUE,            ::rtl::OUString     );
        DECL_PROP0(DESCRIPTION,         ::rtl::OUString     );
        DECL_PROP1(NUMBERFORMAT,        sal_Int32           ,MAYBEVOID);
        DECL_PROP0_BOOL(ISAUTOINCREMENT                     );
        DECL_PROP0_BOOL(ISCURRENCY                      );
        DECL_PROP0_BOOL(HIDDEN                              );
        DECL_PROP0(ISNULLABLE,          sal_Int32           );
        DECL_PROP0_BOOL(ISROWVERSION                        );
        DECL_PROP0(NAME,                ::rtl::OUString     );
        DECL_PROP0(PRECISION,           sal_Int32           );
        DECL_PROP1(RELATIVEPOSITION,    sal_Int32,          MAYBEVOID);
        DECL_PROP0(SCALE,               sal_Int32           );
        DECL_PROP0(TYPE,                sal_Int32           );
        DECL_PROP0(TYPENAME,            ::rtl::OUString     );
        DECL_PROP1(WIDTH,               sal_Int32,          MAYBEVOID);
    END_PROPERTY_HELPER();
}

// cppu::OPropertySetHelper
//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& OTableColumnDescriptor::getInfoHelper()
{
    return *static_cast< ::utl::OPropertyArrayUsageHelper< OTableColumnDescriptor >* >(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
void OTableColumnDescriptor::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
{
    switch (nHandle)
    {
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
        case PROPERTY_ID_ISCURRENCY:
        {
            sal_Bool bVal = m_bCurrency;
            rValue.setValue(&bVal, getBooleanCppuType());
        }   break;
        case PROPERTY_ID_ISROWVERSION:
        {
            sal_Bool bVal = m_bRowVersion;
            rValue.setValue(&bVal, getBooleanCppuType());
        }   break;
        case PROPERTY_ID_NAME:
            OColumn::getFastPropertyValue( rValue, nHandle );
            break;
        default:
            OColumnSettings::getFastPropertyValue( rValue, nHandle );
    }
}

//------------------------------------------------------------------------------
sal_Bool OTableColumnDescriptor::convertFastPropertyValue(
                            Any & rConvertedValue,
                            Any & rOldValue,
                            sal_Int32 nHandle,
                            const Any& rValue )
                                throw (IllegalArgumentException)
{
    sal_Bool bModified = sal_False;
    switch (nHandle)
    {
        case PROPERTY_ID_TYPE:
            bModified = ::utl::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_nType);
            break;
        case PROPERTY_ID_PRECISION:
            bModified = ::utl::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_nPrecision);
            break;
        case PROPERTY_ID_SCALE:
            bModified = ::utl::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_nScale);
            break;
        case PROPERTY_ID_ISNULLABLE:
            bModified = ::utl::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_nIsNullable);
            break;
        case PROPERTY_ID_TYPENAME:
            bModified = ::utl::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aTypeName);
            break;
        case PROPERTY_ID_DESCRIPTION:
            bModified = ::utl::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aDescription);
            break;
        case PROPERTY_ID_DEFAULTVALUE:
            bModified = ::utl::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aDefaultValue);
            break;
        case PROPERTY_ID_ISAUTOINCREMENT:
            bModified = ::utl::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_bAutoIncrement);
            break;
        case PROPERTY_ID_ISCURRENCY:
            bModified = ::utl::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_bCurrency);
            break;
        case PROPERTY_ID_ISROWVERSION:
            bModified = ::utl::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_bRowVersion);
            break;
        case PROPERTY_ID_NAME:
            bModified = OColumn::convertFastPropertyValue( rConvertedValue, rOldValue, nHandle, rValue );
            break;
        default:
            bModified = OColumnSettings::convertFastPropertyValue( rConvertedValue, rOldValue, nHandle, rValue );
    }
    return bModified;
}

//------------------------------------------------------------------------------
void OTableColumnDescriptor::setFastPropertyValue_NoBroadcast(
                                            sal_Int32 nHandle,
                                            const Any& rValue
                                                 )
                                                 throw (Exception)
{
    switch (nHandle)
    {
        case PROPERTY_ID_TYPE:
            OSL_ENSHURE(rValue.getValueType().equals(::getCppuType(static_cast< sal_Int32* >(NULL))),
                "OTableColumnDescriptor::setFastPropertyValue_NoBroadcast(TYPE) : invalid value !");
            rValue >>= m_nType;
            break;
        case PROPERTY_ID_PRECISION:
            OSL_ENSHURE(rValue.getValueType().equals(::getCppuType(static_cast< sal_Int32* >(NULL))),
                "OTableColumnDescriptor::setFastPropertyValue_NoBroadcast(PRECISION) : invalid value !");
            rValue >>= m_nPrecision;
            break;
        case PROPERTY_ID_SCALE:
            OSL_ENSHURE(rValue.getValueType().equals(::getCppuType(static_cast< sal_Int32* >(NULL))),
                "OTableColumnDescriptor::setFastPropertyValue_NoBroadcast(SCALE) : invalid value !");
            rValue >>= m_nScale;
            break;
        case PROPERTY_ID_ISNULLABLE:
            OSL_ENSHURE(rValue.getValueType().equals(::getCppuType(static_cast< sal_Int32* >(NULL))),
                "OTableColumnDescriptor::setFastPropertyValue_NoBroadcast(ISNULLABLE) : invalid value !");
            rValue >>= m_nIsNullable;
            break;
        case PROPERTY_ID_TYPENAME:
            OSL_ENSHURE(rValue.getValueType().equals(::getCppuType(static_cast< rtl::OUString* >(NULL))),
                "OTableColumnDescriptor::setFastPropertyValue_NoBroadcast(TYPENAME) : invalid value !");
            rValue >>= m_aTypeName;
            break;
        case PROPERTY_ID_DESCRIPTION:
            OSL_ENSHURE(rValue.getValueType().equals(::getCppuType(static_cast< rtl::OUString* >(NULL))),
                "OTableColumnDescriptor::setFastPropertyValue_NoBroadcast(DESCRIPTION) : invalid value !");
            rValue >>= m_aDescription;
            break;
        case PROPERTY_ID_DEFAULTVALUE:
            OSL_ENSHURE(rValue.getValueType().equals(::getCppuType(static_cast< rtl::OUString* >(NULL))),
                "OTableColumnDescriptor::setFastPropertyValue_NoBroadcast(DEFAULTVALUE) : invalid value !");
            rValue >>= m_aDefaultValue;
            break;
        case PROPERTY_ID_ISAUTOINCREMENT:
            OSL_ENSHURE(rValue.getValueType().equals(::getCppuType(static_cast< sal_Bool* >(NULL))),
                "OTableColumnDescriptor::setFastPropertyValue_NoBroadcast(DEFAULTVALUE) : invalid value !");
            m_bAutoIncrement = ::utl::getBOOL(rValue);
            break;
        case PROPERTY_ID_ISCURRENCY:
            OSL_ENSHURE(rValue.getValueType().equals(::getCppuType(static_cast< sal_Bool* >(NULL))),
                "OTableColumnDescriptor::setFastPropertyValue_NoBroadcast(DEFAULTVALUE) : invalid value !");
            m_bCurrency = ::utl::getBOOL(rValue);
            break;
        case PROPERTY_ID_ISROWVERSION:
            OSL_ENSHURE(rValue.getValueType().equals(::getCppuType(static_cast< sal_Bool* >(NULL))),
                "OTableColumnDescriptor::setFastPropertyValue_NoBroadcast(ISROWVERSION) : invalid value !");
            m_bRowVersion = ::utl::getBOOL(rValue);
            break;
        case PROPERTY_ID_NAME:
            OColumn::setFastPropertyValue_NoBroadcast( nHandle, rValue );
            break;
        default:
            OColumnSettings::setFastPropertyValue_NoBroadcast( nHandle, rValue );
    }
}

//============================================================
//= OTableColumn
//============================================================
// com::sun::star::lang::XTypeProvider
//--------------------------------------------------------------------------
Sequence< sal_Int8 > OTableColumn::getImplementationId() throw (RuntimeException)
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

// ::com::sun::star::lang::XServiceInfo
//------------------------------------------------------------------------------
rtl::OUString OTableColumn::getImplementationName(  ) throw (RuntimeException)
{
    return rtl::OUString::createFromAscii("com.sun.star.sdb.OTableColumn");
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > OTableColumn::getSupportedServiceNames(  ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aSNS( 2 );
    aSNS[0] = SERVICE_SDBCX_COLUMN;
    aSNS[1] = SERVICE_SDB_COLUMNSETTINGS;
    return aSNS;
}

// utl::OPropertyArrayUsageHelper
//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OTableColumn::createArrayHelper( ) const
{
    BEGIN_PROPERTY_HELPER(17)
        DECL_PROP2(ALIGN,               sal_Int32,          BOUND, MAYBEVOID);
        DECL_PROP1_IFACE(CONTROLMODEL,  XPropertySet ,      BOUND);
        DECL_PROP1(DEFAULTVALUE,        ::rtl::OUString,    READONLY);
        DECL_PROP1(DESCRIPTION,         ::rtl::OUString,    READONLY);
        DECL_PROP2(NUMBERFORMAT,        sal_Int32,          BOUND, MAYBEVOID);
        DECL_PROP1_BOOL(ISAUTOINCREMENT,                    READONLY);
        DECL_PROP1_BOOL(ISCURRENCY,                 READONLY);
        DECL_PROP1_BOOL(HIDDEN,                             BOUND);
        DECL_PROP1(ISNULLABLE,          sal_Int32,          READONLY);
        DECL_PROP1_BOOL(ISROWVERSION,                       READONLY);
        DECL_PROP1(NAME,                ::rtl::OUString,    READONLY);
        DECL_PROP1(PRECISION,           sal_Int32,          READONLY);
        DECL_PROP2(RELATIVEPOSITION,    sal_Int32,          BOUND, MAYBEVOID);
        DECL_PROP1(SCALE,               sal_Int32,          READONLY);
        DECL_PROP1(TYPE,                sal_Int32,          READONLY);
        DECL_PROP1(TYPENAME,            ::rtl::OUString,    READONLY);
        DECL_PROP2(WIDTH,               sal_Int32,          BOUND, MAYBEVOID);
    END_PROPERTY_HELPER();
}

//============================================================
//= OColumnWrapper
//============================================================
//--------------------------------------------------------------------------
OColumnWrapper::OColumnWrapper(const Reference< XPropertySet > & rCol, sal_Int32 nColTypeID)
               :m_xAggregate(rCol)
               ,m_nColTypeID(-1)
{
    // which type of aggregate property do we have
    if (m_nColTypeID == -1)
    {
        // we distingish the properties by the containment of optional properties, these are:
        // Description  0x0001
        // Hidden       0x0002
        // IsRowVersion 0x0004
        Reference <XPropertySetInfo > xInfo(m_xAggregate->getPropertySetInfo());
        m_nColTypeID = 0;
        m_nColTypeID |= xInfo->hasPropertyByName(PROPERTY_DESCRIPTION) ? HAS_DESCRIPTION : 0;
        m_nColTypeID |= xInfo->hasPropertyByName(PROPERTY_DEFAULTVALUE) ? HAS_DEFAULTVALUE : 0;
        m_nColTypeID |= xInfo->hasPropertyByName(PROPERTY_ISROWVERSION) ? HAS_ROWVERSION : 0;
    }
}

//------------------------------------------------------------------------------
void OColumnWrapper::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_NAME:
            rValue <<= m_sName;
            break;
        default:
        {
            // get the property name
            ::rtl::OUString aPropName;
            sal_Int16 nAttributes;
            const_cast<OColumnWrapper*>(this)->getInfoHelper().
                        fillPropertyMembersByHandle(&aPropName, &nAttributes, nHandle);
            OSL_ENSHURE(aPropName.getLength(), "property not found?");

            // now read the value
            rValue = m_xAggregate->getPropertyValue(aPropName);
        }
    }
}

//------------------------------------------------------------------------------
sal_Bool OColumnWrapper::convertFastPropertyValue(
                            Any & rConvertedValue,
                            Any & rOldValue,
                            sal_Int32 nHandle,
                            const Any& rValue )
                                throw (IllegalArgumentException)
{
    // used for the name
    sal_Bool bModified = OColumn::convertFastPropertyValue( rConvertedValue, rOldValue, nHandle, rValue );

    // get the property name
    ::rtl::OUString aPropName;
    sal_Int16 nAttributes;
    getInfoHelper().fillPropertyMembersByHandle(&aPropName, &nAttributes, nHandle);
    OSL_ENSHURE(aPropName.getLength(), "property not found?");

    // now read the value
    m_xAggregate->setPropertyValue(aPropName, rValue);
    return bModified;
}

//------------------------------------------------------------------------------
void OColumnWrapper::setFastPropertyValue_NoBroadcast(
                                                sal_Int32 nHandle,
                                                const Any& rValue
                                                 )
                                                 throw (Exception)
{
    OColumn::setFastPropertyValue_NoBroadcast( nHandle, rValue );
}

//============================================================
//= OTableColumnDescriptorWrapper
//============================================================
// com::sun::star::lang::XTypeProvider
//--------------------------------------------------------------------------
Sequence< sal_Int8 > OTableColumnDescriptorWrapper::getImplementationId() throw (RuntimeException)
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

// ::com::sun::star::lang::XServiceInfo
//------------------------------------------------------------------------------
rtl::OUString OTableColumnDescriptorWrapper::getImplementationName(  ) throw (RuntimeException)
{
    return rtl::OUString::createFromAscii("com.sun.star.sdb.OTableColumnDescriptorWrapper");
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > OTableColumnDescriptorWrapper::getSupportedServiceNames(  ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aSNS( 2 );
    aSNS[0] = SERVICE_SDBCX_COLUMNDESCRIPTOR;
    aSNS[1] = SERVICE_SDB_COLUMNSETTINGS;
    return aSNS;
}

// utl::OPropertyArrayUsageHelper
//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OTableColumnDescriptorWrapper::createArrayHelper( sal_Int32 nId ) const
{
    // BEGIN_PROPERTY_HELPER(17)
    sal_Int32 nPropertyCount = 14;
    // How many properties do we have?
    // Which optional properties are contained?
    if (nId & HAS_DESCRIPTION)
        nPropertyCount++;
    if (nId & HAS_DEFAULTVALUE)
        nPropertyCount++;
    if (nId & HAS_ROWVERSION)
        nPropertyCount++;

    Sequence< Property> aDescriptor(nPropertyCount);
    Property* pDesc = aDescriptor.getArray();
    sal_Int32 nPos = 0;

    //      Description, Defaultvalue, IsRowVersion
        DECL_PROP1(ALIGN,               sal_Int32,          MAYBEVOID);
        DECL_PROP0_IFACE(CONTROLMODEL,  XPropertySet        );
        if (nId & HAS_DEFAULTVALUE)
        {
            DECL_PROP0(DEFAULTVALUE,    ::rtl::OUString );
        }

        if (nId & HAS_DESCRIPTION)
        {
            DECL_PROP0(DESCRIPTION,     ::rtl::OUString );
        }

        DECL_PROP1(NUMBERFORMAT,        sal_Int32,          MAYBEVOID);
        DECL_PROP0_BOOL(ISAUTOINCREMENT                     );
        DECL_PROP0_BOOL(ISCURRENCY                          );
        DECL_PROP0_BOOL(HIDDEN                              );
        DECL_PROP0(ISNULLABLE,          sal_Int32           );

        if (nId & HAS_ROWVERSION)
        {
            DECL_PROP0_BOOL(ISROWVERSION                    );
        }

        DECL_PROP0(NAME,                ::rtl::OUString     );
        DECL_PROP0(PRECISION,           sal_Int32           );
        DECL_PROP1(RELATIVEPOSITION,    sal_Int32,          MAYBEVOID);
        DECL_PROP0(SCALE,               sal_Int32           );
        DECL_PROP0(TYPE,                sal_Int32           );
        DECL_PROP0(TYPENAME,            ::rtl::OUString     );
        DECL_PROP1(WIDTH,               sal_Int32,          MAYBEVOID);
    END_PROPERTY_HELPER();
}

// cppu::OPropertySetHelper
//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& OTableColumnDescriptorWrapper::getInfoHelper()
{
    return *static_cast< OIdPropertyArrayUsageHelper< OTableColumnDescriptorWrapper >* >(this)->getArrayHelper(m_nColTypeID);
}

//------------------------------------------------------------------------------
void OTableColumnDescriptorWrapper::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_ALIGN:
        case PROPERTY_ID_NUMBERFORMAT:
        case PROPERTY_ID_RELATIVEPOSITION:
        case PROPERTY_ID_WIDTH:
        case PROPERTY_ID_HIDDEN:
        case PROPERTY_ID_CONTROLMODEL:
            OColumnSettings::getFastPropertyValue( rValue, nHandle );
            break;
        default:
        {
            // get the property name
            ::rtl::OUString aPropName;
            sal_Int16 nAttributes;
            const_cast<OTableColumnDescriptorWrapper*>(this)->getInfoHelper().
                fillPropertyMembersByHandle(&aPropName, &nAttributes, nHandle);
            OSL_ENSHURE(aPropName.getLength(), "property not found?");

            // now read the value
            rValue = m_xAggregate->getPropertyValue(aPropName);
        }
    }
}

//------------------------------------------------------------------------------
sal_Bool OTableColumnDescriptorWrapper::convertFastPropertyValue(
                            Any & rConvertedValue,
                            Any & rOldValue,
                            sal_Int32 nHandle,
                            const Any& rValue )
                                throw (IllegalArgumentException)
{
    sal_Bool bModified(sal_False);
    switch (nHandle)
    {
        case PROPERTY_ID_ALIGN:
        case PROPERTY_ID_NUMBERFORMAT:
        case PROPERTY_ID_RELATIVEPOSITION:
        case PROPERTY_ID_WIDTH:
        case PROPERTY_ID_HIDDEN:
        case PROPERTY_ID_CONTROLMODEL:
            bModified = OColumnSettings::convertFastPropertyValue( rConvertedValue, rOldValue, nHandle, rValue );
            break;
        default:
            bModified = OColumnWrapper::convertFastPropertyValue( rConvertedValue, rOldValue, nHandle, rValue );
    }
    return bModified;
}

//------------------------------------------------------------------------------
void OTableColumnDescriptorWrapper::setFastPropertyValue_NoBroadcast(
                                                sal_Int32 nHandle,
                                                const Any& rValue
                                                 )
                                                 throw (Exception)
{
    switch (nHandle)
    {
        case PROPERTY_ID_ALIGN:
        case PROPERTY_ID_NUMBERFORMAT:
        case PROPERTY_ID_RELATIVEPOSITION:
        case PROPERTY_ID_WIDTH:
        case PROPERTY_ID_HIDDEN:
        case PROPERTY_ID_CONTROLMODEL:
            OColumnSettings::setFastPropertyValue_NoBroadcast( nHandle, rValue );
            break;
        default:
            OColumnWrapper::setFastPropertyValue_NoBroadcast( nHandle, rValue );
    }
}

//============================================================
//= OTableColumnWrapper
//============================================================
// com::sun::star::lang::XTypeProvider
//--------------------------------------------------------------------------
Sequence< sal_Int8 > OTableColumnWrapper::getImplementationId() throw (RuntimeException)
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
// ::com::sun::star::lang::XServiceInfo
//------------------------------------------------------------------------------
rtl::OUString OTableColumnWrapper::getImplementationName(  ) throw (RuntimeException)
{
    return rtl::OUString::createFromAscii("com.sun.star.sdb.OTableColumnWrapper");
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > OTableColumnWrapper::getSupportedServiceNames(  ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aSNS( 2 );
    aSNS[0] = SERVICE_SDBCX_COLUMN;
    aSNS[1] = SERVICE_SDB_COLUMNSETTINGS;
    return aSNS;
}

// utl::OPropertyArrayUsageHelper
//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OTableColumnWrapper::createArrayHelper( sal_Int32 nId ) const
{
    // BEGIN_PROPERTY_HELPER(17)
    sal_Int32 nPropertyCount = 14;
    // How many properties do we have?
    // Which optional properties are contained?
    if (nId & HAS_DESCRIPTION)
        nPropertyCount++;
    if (nId & HAS_DEFAULTVALUE)
        nPropertyCount++;
    if (nId & HAS_ROWVERSION)
        nPropertyCount++;

    Sequence< Property> aDescriptor(nPropertyCount);
    Property* pDesc = aDescriptor.getArray();
    sal_Int32 nPos = 0;

    //      Description, Defaultvalue, IsRowVersion
        DECL_PROP2(ALIGN,               sal_Int32,          BOUND, MAYBEVOID);
        DECL_PROP1_IFACE(CONTROLMODEL,  XPropertySet ,      BOUND);
        if (nId & HAS_DEFAULTVALUE)
        {
            DECL_PROP1(DEFAULTVALUE,        ::rtl::OUString,    READONLY);
        }

        if (nId & HAS_DESCRIPTION)
        {
            DECL_PROP1(DESCRIPTION,         ::rtl::OUString,    READONLY);
        }

        DECL_PROP2(NUMBERFORMAT,        sal_Int32,          BOUND, MAYBEVOID);
        DECL_PROP1_BOOL(ISAUTOINCREMENT,                    READONLY);
        DECL_PROP1_BOOL(ISCURRENCY,                         READONLY);
        DECL_PROP1_BOOL(HIDDEN,                             BOUND);
        DECL_PROP1(ISNULLABLE,          sal_Int32,          READONLY);

        if (nId & HAS_ROWVERSION)
        {
            DECL_PROP1_BOOL(ISROWVERSION,                       READONLY);
        }

        DECL_PROP1(NAME,                ::rtl::OUString,    READONLY);
        DECL_PROP1(PRECISION,           sal_Int32,          READONLY);
        DECL_PROP2(RELATIVEPOSITION,    sal_Int32,          BOUND, MAYBEVOID);
        DECL_PROP1(SCALE,               sal_Int32,          READONLY);
        DECL_PROP1(TYPE,                sal_Int32,          READONLY);
        DECL_PROP1(TYPENAME,            ::rtl::OUString,    READONLY);
        DECL_PROP2(WIDTH,               sal_Int32,          BOUND, MAYBEVOID);
    END_PROPERTY_HELPER();
}

//============================================================
//= OIndexColumnWrapper
//============================================================
// com::sun::star::lang::XTypeProvider
//--------------------------------------------------------------------------
Sequence< sal_Int8 > OIndexColumnWrapper::getImplementationId() throw (RuntimeException)
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
// ::com::sun::star::lang::XServiceInfo
//------------------------------------------------------------------------------
rtl::OUString OIndexColumnWrapper::getImplementationName(  ) throw (RuntimeException)
{
    return rtl::OUString::createFromAscii("com.sun.star.sdb.OIndexColumnWrapper");
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > OIndexColumnWrapper::getSupportedServiceNames(  ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aSNS( 2 );
    aSNS[0] = SERVICE_SDBCX_COLUMN;
    aSNS[1] = SERVICE_SDBCX_INDEXCOLUMN;
    return aSNS;
}

// utl::OPropertyArrayUsageHelper
//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OIndexColumnWrapper::createArrayHelper( sal_Int32 nId ) const
{
    BEGIN_PROPERTY_HELPER(9)
        DECL_PROP1_BOOL(ISASCENDING,                    READONLY);
        DECL_PROP1_BOOL(ISAUTOINCREMENT,                    READONLY);
        DECL_PROP1_BOOL(ISCURRENCY,                 READONLY);
        DECL_PROP1(ISNULLABLE,          sal_Int32,          READONLY);
        DECL_PROP1(NAME,                ::rtl::OUString,    READONLY);
        DECL_PROP1(PRECISION,           sal_Int32,          READONLY);
        DECL_PROP1(SCALE,               sal_Int32,          READONLY);
        DECL_PROP1(TYPE,                sal_Int32,          READONLY);
        DECL_PROP1(TYPENAME,            ::rtl::OUString,    READONLY);
    END_PROPERTY_HELPER();
}

//------------------------------------------------------------------------------
void OIndexColumnWrapper::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_ISASCENDING:
        {
            sal_Bool bVal = m_bAscending;
            rValue.setValue(&bVal, getBooleanCppuType());
        }   break;
        default:
            OColumnWrapper::getFastPropertyValue( rValue, nHandle );
    }
}

//============================================================
//= OKeyColumnWrapper
//============================================================
// com::sun::star::lang::XTypeProvider
//--------------------------------------------------------------------------
Sequence< sal_Int8 > OKeyColumnWrapper::getImplementationId() throw (RuntimeException)
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
// ::com::sun::star::lang::XServiceInfo
//------------------------------------------------------------------------------
rtl::OUString OKeyColumnWrapper::getImplementationName(  ) throw (RuntimeException)
{
    return rtl::OUString::createFromAscii("com.sun.star.sdb.OIndexColumnWrapper");
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > OKeyColumnWrapper::getSupportedServiceNames(  ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aSNS( 2 );
    aSNS[0] = SERVICE_SDBCX_COLUMN;
    aSNS[1] = SERVICE_SDBCX_KEYCOLUMN;
    return aSNS;
}

// utl::OPropertyArrayUsageHelper
//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OKeyColumnWrapper::createArrayHelper( sal_Int32 nId ) const
{
    BEGIN_PROPERTY_HELPER(9)
        DECL_PROP1_BOOL(ISAUTOINCREMENT,                    READONLY);
        DECL_PROP1_BOOL(ISCURRENCY,                 READONLY);
        DECL_PROP1(ISNULLABLE,          sal_Int32,          READONLY);
        DECL_PROP1(NAME,                ::rtl::OUString,    READONLY);
        DECL_PROP1(PRECISION,           sal_Int32,          READONLY);
        DECL_PROP1(RELATEDCOLUMN,       ::rtl::OUString,    READONLY);
        DECL_PROP1(SCALE,               sal_Int32,          READONLY);
        DECL_PROP1(TYPE,                sal_Int32,          READONLY);
        DECL_PROP1(TYPENAME,            ::rtl::OUString,    READONLY);
    END_PROPERTY_HELPER();
}

//------------------------------------------------------------------------------
void OKeyColumnWrapper::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_RELATEDCOLUMN:
            rValue <<= m_aRelatedColumn;
            break;
        default:
            OColumnWrapper::getFastPropertyValue( rValue, nHandle );
    }
}


