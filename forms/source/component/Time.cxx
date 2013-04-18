/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "Time.hxx"
#include <tools/debug.hxx>
#include <tools/time.hxx>
#include <connectivity/dbconversion.hxx>
#include <com/sun/star/sdbc/DataType.hpp>
#include <comphelper/processfactory.hxx>

using namespace dbtools;

//.........................................................................
namespace frm
{
//.........................................................................

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;

//==================================================================
//=
//==================================================================

//==================================================================
//= OTimeControl
//==================================================================
//------------------------------------------------------------------
OTimeControl::OTimeControl(const Reference<XMultiServiceFactory>& _rxFactory)
               :OBoundControl(_rxFactory, VCL_CONTROL_TIMEFIELD)
{
}

//------------------------------------------------------------------
InterfaceRef SAL_CALL OTimeControl_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return *(new OTimeControl(_rxFactory));
}

//------------------------------------------------------------------------------
Sequence<Type> OTimeControl::_getTypes()
{
    return OBoundControl::_getTypes();
}

//------------------------------------------------------------------------------
StringSequence SAL_CALL OTimeControl::getSupportedServiceNames() throw()
{
    StringSequence aSupported = OBoundControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_CONTROL_TIMEFIELD;
    return aSupported;
}

//==================================================================
//= OTimeModel
//==================================================================
//------------------------------------------------------------------
InterfaceRef SAL_CALL OTimeModel_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return *(new OTimeModel(_rxFactory));
}

// XServiceInfo
//------------------------------------------------------------------------------
StringSequence SAL_CALL OTimeModel::getSupportedServiceNames() throw()
{
    StringSequence aSupported = OBoundControlModel::getSupportedServiceNames();

    sal_Int32 nOldLen = aSupported.getLength();
    aSupported.realloc( nOldLen + 8 );
    OUString* pStoreTo = aSupported.getArray() + nOldLen;

    *pStoreTo++ = BINDABLE_CONTROL_MODEL;
    *pStoreTo++ = DATA_AWARE_CONTROL_MODEL;
    *pStoreTo++ = VALIDATABLE_CONTROL_MODEL;

    *pStoreTo++ = BINDABLE_DATA_AWARE_CONTROL_MODEL;
    *pStoreTo++ = VALIDATABLE_BINDABLE_CONTROL_MODEL;

    *pStoreTo++ = FRM_SUN_COMPONENT_TIMEFIELD;
    *pStoreTo++ = FRM_SUN_COMPONENT_DATABASE_TIMEFIELD;
    *pStoreTo++ = BINDABLE_DATABASE_TIME_FIELD;

    return aSupported;
}

//------------------------------------------------------------------------------
Sequence<Type> OTimeModel::_getTypes()
{
    return OBoundControlModel::_getTypes();
}

//------------------------------------------------------------------
DBG_NAME( OTimeModel )
//------------------------------------------------------------------
OTimeModel::OTimeModel(const Reference<XMultiServiceFactory>& _rxFactory)
            :OEditBaseModel( _rxFactory, VCL_CONTROLMODEL_TIMEFIELD, FRM_SUN_CONTROL_TIMEFIELD, sal_True, sal_True )
                                    // use the old control name for compytibility reasons
            ,OLimitedFormats( comphelper::getComponentContext(_rxFactory), FormComponentType::TIMEFIELD)
{
    DBG_CTOR( OTimeModel, NULL );

    m_nClassId = FormComponentType::TIMEFIELD;
    initValueProperty( PROPERTY_TIME, PROPERTY_ID_TIME );

    setAggregateSet(m_xAggregateFastSet, getOriginalHandle(PROPERTY_ID_TIMEFORMAT));
}

//------------------------------------------------------------------------------
OTimeModel::OTimeModel( const OTimeModel* _pOriginal, const Reference<XMultiServiceFactory>& _rxFactory )
    :OEditBaseModel( _pOriginal, _rxFactory )
    ,OLimitedFormats( comphelper::getComponentContext(_rxFactory), FormComponentType::TIMEFIELD )
{
    DBG_CTOR( OTimeModel, NULL );

    setAggregateSet( m_xAggregateFastSet, getOriginalHandle( PROPERTY_ID_TIMEFORMAT ) );
}

//------------------------------------------------------------------------------
OTimeModel::~OTimeModel( )
{
    setAggregateSet(Reference< XFastPropertySet >(), -1);
    DBG_DTOR( OTimeModel, NULL );
}

// XCloneable
//------------------------------------------------------------------------------
IMPLEMENT_DEFAULT_CLONING( OTimeModel )

//------------------------------------------------------------------------------
OUString SAL_CALL OTimeModel::getServiceName() throw ( ::com::sun::star::uno::RuntimeException)
{
    return FRM_COMPONENT_TIMEFIELD; // old (non-sun) name for compatibility !
}

// XPropertySet
//------------------------------------------------------------------------------
void OTimeModel::describeFixedProperties( Sequence< Property >& _rProps ) const
{
    BEGIN_DESCRIBE_PROPERTIES( 4, OEditBaseModel )
        DECL_PROP3(DEFAULT_TIME,            sal_Int32,              BOUND, MAYBEDEFAULT, MAYBEVOID);
        DECL_PROP1(TABINDEX,                sal_Int16,              BOUND);
        DECL_PROP1(FORMATKEY,               sal_Int32,              TRANSIENT);
        DECL_IFACE_PROP2(FORMATSSUPPLIER,   XNumberFormatsSupplier, READONLY, TRANSIENT);
    END_DESCRIBE_PROPERTIES();
}

//------------------------------------------------------------------------------
void SAL_CALL OTimeModel::getFastPropertyValue(Any& _rValue, sal_Int32 _nHandle ) const
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
sal_Bool SAL_CALL OTimeModel::convertFastPropertyValue(Any& _rConvertedValue, Any& _rOldValue,
        sal_Int32 _nHandle, const Any& _rValue ) throw(IllegalArgumentException)
{
    if (PROPERTY_ID_FORMATKEY == _nHandle)
        return convertFormatKeyPropertyValue(_rConvertedValue, _rOldValue, _rValue);
    else
        return OEditBaseModel::convertFastPropertyValue(_rConvertedValue, _rOldValue, _nHandle, _rValue );
}

//------------------------------------------------------------------------------
void SAL_CALL OTimeModel::setFastPropertyValue_NoBroadcast(sal_Int32 _nHandle, const Any& _rValue) throw ( ::com::sun::star::uno::Exception)
{
    if (PROPERTY_ID_FORMATKEY == _nHandle)
        setFormatKeyPropertyValue(_rValue);
    else
        OEditBaseModel::setFastPropertyValue_NoBroadcast(_nHandle, _rValue);
}

// XLoadListener
//------------------------------------------------------------------------------
void OTimeModel::onConnectedDbColumn( const Reference< XInterface >& _rxForm )
{
    OBoundControlModel::onConnectedDbColumn( _rxForm );
    Reference<XPropertySet> xField = getField();
    if (xField.is())
    {
        m_bDateTimeField = sal_False;
        try
        {
            sal_Int32 nFieldType = 0;
            xField->getPropertyValue(PROPERTY_FIELDTYPE) >>= nFieldType;
            m_bDateTimeField = (nFieldType == DataType::TIMESTAMP);
        }
        catch(const Exception&)
        {
        }
    }
}

//------------------------------------------------------------------------------
sal_Bool OTimeModel::commitControlValueToDbColumn( bool /*_bPostReset*/ )
{
    Any aControlValue( m_xAggregateFastSet->getFastPropertyValue( getValuePropertyAggHandle() ) );
    if ( !compare( aControlValue, m_aSaveValue ) )
    {
        if ( !aControlValue.hasValue() )
            m_xColumnUpdate->updateNull();
        else
        {
            try
            {
                util::Time aTime;
                if ( !( aControlValue >>= aTime ) )
                {
                    sal_Int64 nAsInt(0);
                    aControlValue >>= nAsInt;
                    aTime = DBTypeConversion::toTime(nAsInt);
                }

                if (!m_bDateTimeField)
                    m_xColumnUpdate->updateTime(aTime);
                else
                {
                    util::DateTime aDateTime = m_xColumn->getTimestamp();
                    aDateTime.NanoSeconds = aTime.NanoSeconds;
                    aDateTime.Seconds = aTime.Seconds;
                    aDateTime.Minutes = aTime.Minutes;
                    aDateTime.Hours = aTime.Hours;
                    m_xColumnUpdate->updateTimestamp(aDateTime);
                }
            }
            catch(const Exception&)
            {
                return sal_False;
            }
        }
        m_aSaveValue = aControlValue;
    }
    return sal_True;
}

//------------------------------------------------------------------------------
void OTimeModel::impl_translateControlValueToUNOTime( Any& _rUNOValue ) const
{
    _rUNOValue = getControlValue();
    if ( _rUNOValue.hasValue() )
    {
        sal_Int64 nTime = 0;
        OSL_VERIFY( _rUNOValue >>= nTime );
        if ( nTime == ::Time( 99, 99, 99 ).GetTime() )
            // "invalid time" in VCL is different from "invalid time" in UNO
            _rUNOValue.clear();
        else
            _rUNOValue <<= DBTypeConversion::toTime( nTime );
    }
}

//------------------------------------------------------------------------------
Any OTimeModel::translateControlValueToExternalValue( ) const
{
    Any aExternalValue;
    impl_translateControlValueToUNOTime( aExternalValue );
    return aExternalValue;
}

//------------------------------------------------------------------------------
Any OTimeModel::translateExternalValueToControlValue( const Any& _rExternalValue ) const
{
    Any aControlValue;
    if ( _rExternalValue.hasValue() )
    {
        util::Time aTime;
        OSL_VERIFY( _rExternalValue >>= aTime );
        aControlValue <<= DBTypeConversion::toINT64( aTime );
    }
    return aControlValue;
}

//------------------------------------------------------------------------------
Any OTimeModel::translateControlValueToValidatableValue( ) const
{
    Any aValidatableValue;
    impl_translateControlValueToUNOTime( aValidatableValue );
    return aValidatableValue;
}

//------------------------------------------------------------------------------
Any OTimeModel::translateDbColumnToControlValue()
{
    util::Time aTime = m_xColumn->getTime();
    if ( m_xColumn->wasNull() )
        m_aSaveValue.clear();
    else
        // TODO FIXME: "the aggregated set expects an Int32 as value ..."
        //             need to fix it for int64
        m_aSaveValue <<= DBTypeConversion::toINT64( aTime );

    return m_aSaveValue;
}

//------------------------------------------------------------------------------
Any OTimeModel::getDefaultForReset() const
{
    return m_aDefault;
}

//------------------------------------------------------------------------------
void OTimeModel::resetNoBroadcast()
{
    OEditBaseModel::resetNoBroadcast();
    m_aSaveValue.clear();
}

//------------------------------------------------------------------------------
Sequence< Type > OTimeModel::getSupportedBindingTypes()
{
    return Sequence< Type >( &::getCppuType( static_cast< util::Time* >( NULL ) ), 1 );
}

//.........................................................................
}   // namespace frm
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
