/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Time.cxx,v $
 * $Revision: 1.26 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_forms.hxx"
#include "Time.hxx"
#include <tools/debug.hxx>
#include <tools/time.hxx>
#include <connectivity/dbconversion.hxx>
#include <com/sun/star/sdbc/DataType.hpp>

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

    ::rtl::OUString*pArray = aSupported.getArray();
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
    ::rtl::OUString* pStoreTo = aSupported.getArray() + nOldLen;

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
            ,OLimitedFormats(_rxFactory, FormComponentType::TIMEFIELD)
{
    DBG_CTOR( OTimeModel, NULL );

    m_nClassId = FormComponentType::TIMEFIELD;
    initValueProperty( PROPERTY_TIME, PROPERTY_ID_TIME );

    setAggregateSet(m_xAggregateFastSet, getOriginalHandle(PROPERTY_ID_TIMEFORMAT));
}

//------------------------------------------------------------------------------
OTimeModel::OTimeModel( const OTimeModel* _pOriginal, const Reference<XMultiServiceFactory>& _rxFactory )
    :OEditBaseModel( _pOriginal, _rxFactory )
    ,OLimitedFormats( _rxFactory, FormComponentType::TIMEFIELD )
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
::rtl::OUString SAL_CALL OTimeModel::getServiceName() throw ( ::com::sun::star::uno::RuntimeException)
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
        catch(Exception&)
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
                    sal_Int32 nAsInt(0);
                    aControlValue >>= nAsInt;
                    aTime = DBTypeConversion::toTime(nAsInt);
                }

                if (!m_bDateTimeField)
                    m_xColumnUpdate->updateTime(aTime);
                else
                {
                    util::DateTime aDateTime = m_xColumn->getTimestamp();
                    aDateTime.HundredthSeconds = aTime.HundredthSeconds;
                    aDateTime.Seconds = aTime.Seconds;
                    aDateTime.Minutes = aTime.Minutes;
                    aDateTime.Hours = aTime.Hours;
                    m_xColumnUpdate->updateTimestamp(aDateTime);
                }
            }
            catch(Exception&)
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
        sal_Int32 nTime = 0;
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
Any OTimeModel::translateExternalValueToControlValue( ) const
{
    OSL_PRECOND( hasExternalValueBinding(),
        "OTimeModel::translateExternalValueToControlValue: precondition not met!" );

    Any aControlValue;
    if ( hasExternalValueBinding() )
    {
        Any aExternalValue = getExternalValueBinding()->getValue( ::getCppuType( static_cast< util::Time* >( NULL ) ) );
        if ( aExternalValue.hasValue() )
        {
            util::Time aTime;
            OSL_VERIFY( aExternalValue >>= aTime );
            aControlValue <<= DBTypeConversion::toINT32( aTime );
        }
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
        // the aggregated set expects an Int32 as value ...
        m_aSaveValue <<= DBTypeConversion::toINT32( aTime );

    return m_aSaveValue;
}

//------------------------------------------------------------------------------
Any OTimeModel::getDefaultForReset() const
{
    return m_aDefault;
}

//------------------------------------------------------------------------------
sal_Bool OTimeModel::approveValueBinding( const Reference< binding::XValueBinding >& _rxBinding )
{
    OSL_PRECOND( _rxBinding.is(), "OTimeModel::approveValueBinding: invalid binding!" );

    return  _rxBinding.is()
        &&  _rxBinding->supportsType( ::getCppuType( static_cast< util::Time* >( NULL ) ) );
}

//.........................................................................
}   // namespace frm
//.........................................................................

