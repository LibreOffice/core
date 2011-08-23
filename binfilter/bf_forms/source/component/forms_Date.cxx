/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <comphelper/proparrhlp.hxx>

#include "Date.hxx"

#include <tools/date.hxx>
#include <connectivity/dbconversion.hxx>

#include <com/sun/star/sdbc/DataType.hpp>

#include <com/sun/star/form/FormComponentType.hpp>

#include <com/sun/star/beans/PropertyAttribute.hpp>


#include "services.hxx"

namespace binfilter {

using namespace dbtools;

//.........................................................................
namespace frm
{
//.........................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;

//------------------------------------------------------------------
ODateControl::ODateControl(const Reference<XMultiServiceFactory>& _rxFactory)
               :OBoundControl(_rxFactory, VCL_CONTROL_DATEFIELD)
{
}

//------------------------------------------------------------------
InterfaceRef SAL_CALL ODateControl_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return *(new ODateControl(_rxFactory));
}

//------------------------------------------------------------------------------
Sequence<Type> ODateControl::_getTypes()
{
    return OBoundControl::_getTypes();
}

//------------------------------------------------------------------------------
StringSequence SAL_CALL ODateControl::getSupportedServiceNames() throw()
{
    StringSequence aSupported = OBoundControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    ::rtl::OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_CONTROL_DATEFIELD;
    return aSupported;
}

/*************************************************************************/
sal_Int32	ODateModel::nDateHandle = -1;
//------------------------------------------------------------------
InterfaceRef SAL_CALL ODateModel_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return *(new ODateModel(_rxFactory));
}

//------------------------------------------------------------------------------
Sequence<Type> ODateModel::_getTypes()
{
    return OEditBaseModel::_getTypes();
}

//------------------------------------------------------------------
DBG_NAME( ODateModel )
//------------------------------------------------------------------
ODateModel::ODateModel(const Reference<XMultiServiceFactory>& _rxFactory)
            :OEditBaseModel(_rxFactory, VCL_CONTROLMODEL_DATEFIELD, FRM_CONTROL_DATEFIELD )
                        // use the old control name for compytibility reasons
            ,OLimitedFormats(_rxFactory, FormComponentType::DATEFIELD)
{
    DBG_CTOR( ODateModel, NULL );

    m_nClassId = FormComponentType::DATEFIELD;
    m_sDataFieldConnectivityProperty = PROPERTY_DATE;
    if (ODateModel::nDateHandle == -1)
        ODateModel::nDateHandle = getOriginalHandle(PROPERTY_ID_DATE);

    setAggregateSet(m_xAggregateFastSet, getOriginalHandle(PROPERTY_ID_DATEFORMAT));
}

//------------------------------------------------------------------------------
ODateModel::ODateModel( const ODateModel* _pOriginal, const Reference<XMultiServiceFactory>& _rxFactory )
    :OEditBaseModel( _pOriginal, _rxFactory )
    ,OLimitedFormats( _rxFactory, FormComponentType::DATEFIELD )
{
    DBG_CTOR( ODateModel, NULL );

    setAggregateSet( m_xAggregateFastSet, getOriginalHandle( PROPERTY_ID_DATEFORMAT ) );
}

//------------------------------------------------------------------------------
ODateModel::~ODateModel( )
{
    setAggregateSet(Reference< XFastPropertySet >(), -1);
    DBG_DTOR( ODateModel, NULL );
}

// XCloneable
//------------------------------------------------------------------------------
IMPLEMENT_DEFAULT_CLONING( ODateModel )

// XServiceInfo
//------------------------------------------------------------------------------
StringSequence SAL_CALL ODateModel::getSupportedServiceNames() throw()
{
    StringSequence aSupported = OBoundControlModel::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 2);

    ::rtl::OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-2] = FRM_SUN_COMPONENT_DATABASE_DATEFIELD;
    pArray[aSupported.getLength()-1] = FRM_SUN_COMPONENT_DATEFIELD;
    return aSupported;
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODateModel::getServiceName() throw ( ::com::sun::star::uno::RuntimeException)
{
    return FRM_COMPONENT_DATEFIELD;	// old (non-sun) name for compatibility !
}

// XPropertySet
//------------------------------------------------------------------------------
Reference<XPropertySetInfo> SAL_CALL ODateModel::getPropertySetInfo() throw( RuntimeException )
{
    Reference<XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//------------------------------------------------------------------------------
void ODateModel::fillProperties(
        Sequence< Property >& _rProps,
        Sequence< Property >& _rAggregateProps ) const
{
    FRM_BEGIN_PROP_HELPER(11)
        DECL_PROP1(NAME,					::rtl::OUString,		BOUND);
        DECL_PROP2(CLASSID,					sal_Int16,				READONLY, TRANSIENT);
        DECL_PROP3(DEFAULT_DATE,			sal_Int32,				BOUND, MAYBEDEFAULT, MAYBEVOID);
        DECL_PROP1(TAG,						::rtl::OUString,		BOUND);
        DECL_PROP1(TABINDEX,				sal_Int16,				BOUND);
        DECL_PROP1(CONTROLSOURCE,			::rtl::OUString,		BOUND);
        DECL_IFACE_PROP3(BOUNDFIELD,		XPropertySet,			BOUND,READONLY, TRANSIENT);
        DECL_IFACE_PROP2(CONTROLLABEL,		XPropertySet,			BOUND, MAYBEVOID);
        DECL_PROP2(CONTROLSOURCEPROPERTY,	::rtl::OUString,		READONLY, TRANSIENT);
        DECL_PROP1(FORMATKEY,				sal_Int32,				TRANSIENT);
        DECL_IFACE_PROP2(FORMATSSUPPLIER,	XNumberFormatsSupplier,	READONLY, TRANSIENT);
    FRM_END_PROP_HELPER();
}

//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& ODateModel::getInfoHelper()
{
    return *const_cast<ODateModel*>(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
void SAL_CALL ODateModel::getFastPropertyValue(Any& _rValue, sal_Int32 _nHandle ) const
{
    switch (_nHandle)
    {
        case PROPERTY_ID_FORMATKEY:
            getFormatKeyPropertyValue(_rValue);
            break;
        case PROPERTY_ID_FORMATSSUPPLIER:
            _rValue <<= getFormatsSupplier();
            break;
        default:
            OEditBaseModel::getFastPropertyValue(_rValue, _nHandle);
            break;
    }
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL ODateModel::convertFastPropertyValue(Any& _rConvertedValue, Any& _rOldValue,
        sal_Int32 _nHandle, const Any& _rValue ) throw(IllegalArgumentException)
{
    if (PROPERTY_ID_FORMATKEY == _nHandle)
        return convertFormatKeyPropertyValue(_rConvertedValue, _rOldValue, _rValue);
    else
        return OEditBaseModel::convertFastPropertyValue(_rConvertedValue, _rOldValue, _nHandle, _rValue );
}

//------------------------------------------------------------------------------
void SAL_CALL ODateModel::setFastPropertyValue_NoBroadcast(sal_Int32 _nHandle, const Any& _rValue) throw ( ::com::sun::star::uno::Exception)
{
    if (PROPERTY_ID_FORMATKEY == _nHandle)
        setFormatKeyPropertyValue(_rValue);
    else
        OEditBaseModel::setFastPropertyValue_NoBroadcast(_nHandle, _rValue);
}

// XLoadListener
//------------------------------------------------------------------------------
void ODateModel::_loaded(const EventObject& rEvent)
{
    OBoundControlModel::_loaded(rEvent);
    Reference<XPropertySet> xField = getField();
    if (xField.is())
    {
        m_bDateTimeField = sal_False;
        try
        {
            sal_Int32 nFieldType;
            xField->getPropertyValue(PROPERTY_FIELDTYPE) >>= nFieldType;
            m_bDateTimeField = (nFieldType == DataType::TIMESTAMP);
        }
        catch(Exception&)
        {
        }
    }
}

// XBoundComponent
//------------------------------------------------------------------------------
sal_Bool ODateModel::_commit()
{
    Any aNewValue = m_xAggregateFastSet->getFastPropertyValue( ODateModel::nDateHandle );
    if (!compare(aNewValue, m_aSaveValue))
    {
        if (!aNewValue.hasValue())
            m_xColumnUpdate->updateNull();
        else
        {
            try
            {
                util::Date aDate;
                if (!(aNewValue >>= aDate))
                {
                    sal_Int32 nAsInt(0);
                    aNewValue >>= nAsInt;
                    aDate = DBTypeConversion::toDate(nAsInt);
                }

                if (!m_bDateTimeField)
                    m_xColumnUpdate->updateDate(aDate);
                else
                {
                    util::DateTime aDateTime = m_xColumn->getTimestamp();
                    aDateTime.Day = aDate.Day;
                    aDateTime.Month = aDate.Month;
                    aDateTime.Year = aDate.Year;
                    m_xColumnUpdate->updateTimestamp(aDateTime);
                }
            }
            catch(Exception&)
            {
                return sal_False;
            }
        }
        m_aSaveValue = aNewValue;
    }
    return sal_True;
}

//------------------------------------------------------------------------------
void ODateModel::_onValueChanged()
{
    util::Date aDate = m_xColumn->getDate();
    if (m_xColumn->wasNull())
        m_aSaveValue.clear();
    else
        // the aggregated set expects an Int32 as value ...
        m_aSaveValue <<= DBTypeConversion::toINT32(aDate);

    {	// release our mutex once (it's acquired in the calling method !), as setting aggregate properties
        // may cause any uno controls belonging to us to lock the solar mutex, which is potentially dangerous with
        // our own mutex locked
        // FS - 72451 - 31.01.00
        MutexRelease aRelease(m_aMutex);
        m_xAggregateFastSet->setFastPropertyValue(ODateModel::nDateHandle, m_aSaveValue);
    }
}

//------------------------------------------------------------------------------
void ODateModel::_reset()
{
    Any aValue;
    if (m_aDefault.getValueType().getTypeClass() == TypeClass_LONG)
        aValue = m_aDefault;
    else
    {	// aktuelles Datum einstellen
        ::Date aCurrentDate;
        aValue <<= (sal_Int32)aCurrentDate.GetDate();
    }

    {	// release our mutex once (it's acquired in the calling method !), as setting aggregate properties
        // may cause any uno controls belonging to us to lock the solar mutex, which is potentially dangerous with
        // our own mutex locked
        // FS - 72451 - 31.01.00
        MutexRelease aRelease(m_aMutex);
        m_xAggregateFastSet->setFastPropertyValue(ODateModel::nDateHandle, aValue);
    }
}

//.........................................................................
}	// namespace frm
//.........................................................................

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
