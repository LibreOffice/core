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

#include "Date.hxx"
#include <tools/debug.hxx>
#include <tools/date.hxx>
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
            :OEditBaseModel( _rxFactory, VCL_CONTROLMODEL_DATEFIELD, FRM_SUN_CONTROL_DATEFIELD, sal_True, sal_True )
                        // use the old control name for compytibility reasons
            ,OLimitedFormats( _rxFactory, FormComponentType::DATEFIELD )
{
    DBG_CTOR( ODateModel, NULL );

    m_nClassId = FormComponentType::DATEFIELD;
    initValueProperty( PROPERTY_DATE, PROPERTY_ID_DATE );

    setAggregateSet(m_xAggregateFastSet, getOriginalHandle(PROPERTY_ID_DATEFORMAT));

    osl_atomic_increment( &m_refCount );
    try
    {
        if ( m_xAggregateSet.is() )
            m_xAggregateSet->setPropertyValue( PROPERTY_DATEMIN, makeAny( (sal_Int32)( ::Date( 1, 1, 1800 ).GetDate() ) ) );
    }
    catch( const Exception& )
    {
        OSL_FAIL( "ODateModel::ODateModel: caught an exception!" );
    }
    osl_atomic_decrement( &m_refCount );
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

    sal_Int32 nOldLen = aSupported.getLength();
    aSupported.realloc( nOldLen + 8 );
    ::rtl::OUString* pStoreTo = aSupported.getArray() + nOldLen;

    *pStoreTo++ = BINDABLE_CONTROL_MODEL;
    *pStoreTo++ = DATA_AWARE_CONTROL_MODEL;
    *pStoreTo++ = VALIDATABLE_CONTROL_MODEL;

    *pStoreTo++ = BINDABLE_DATA_AWARE_CONTROL_MODEL;
    *pStoreTo++ = VALIDATABLE_BINDABLE_CONTROL_MODEL;

    *pStoreTo++ = FRM_SUN_COMPONENT_DATEFIELD;
    *pStoreTo++ = FRM_SUN_COMPONENT_DATABASE_DATEFIELD;
    *pStoreTo++ = BINDABLE_DATABASE_DATE_FIELD;

    return aSupported;
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODateModel::getServiceName() throw ( ::com::sun::star::uno::RuntimeException)
{
    return FRM_COMPONENT_DATEFIELD; // old (non-sun) name for compatibility !
}

// XPropertySet
//------------------------------------------------------------------------------
void ODateModel::describeFixedProperties( Sequence< Property >& _rProps ) const
{
    BEGIN_DESCRIBE_PROPERTIES( 4, OEditBaseModel )
        DECL_PROP3(DEFAULT_DATE,            sal_Int32,              BOUND, MAYBEDEFAULT, MAYBEVOID);
        DECL_PROP1(TABINDEX,                sal_Int16,              BOUND);
        DECL_PROP1(FORMATKEY,               sal_Int32,              TRANSIENT);
        DECL_IFACE_PROP2(FORMATSSUPPLIER,   XNumberFormatsSupplier, READONLY, TRANSIENT);
    END_DESCRIBE_PROPERTIES();
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
void ODateModel::onConnectedDbColumn( const Reference< XInterface >& _rxForm )
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
sal_Bool ODateModel::commitControlValueToDbColumn( bool /*_bPostReset*/ )
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
                util::Date aDate;
                if ( !( aControlValue >>= aDate ) )
                {
                    sal_Int32 nAsInt(0);
                    aControlValue >>= nAsInt;
                    aDate = DBTypeConversion::toDate(nAsInt);
                }

                if ( !m_bDateTimeField )
                    m_xColumnUpdate->updateDate( aDate );
                else
                {
                    util::DateTime aDateTime = m_xColumn->getTimestamp();
                    aDateTime.Day = aDate.Day;
                    aDateTime.Month = aDate.Month;
                    aDateTime.Year = aDate.Year;
                    m_xColumnUpdate->updateTimestamp( aDateTime );
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
void ODateModel::impl_translateControlValueToUNODate( Any& _rUNOValue ) const
{
    _rUNOValue = getControlValue();
    if ( _rUNOValue.hasValue() )
    {
        sal_Int32 nDate = 0;
        OSL_VERIFY( _rUNOValue >>= nDate );
        _rUNOValue <<= DBTypeConversion::toDate( nDate );
    }
}

//------------------------------------------------------------------------------
Any ODateModel::translateControlValueToExternalValue( ) const
{
    Any aExternalValue;
    impl_translateControlValueToUNODate( aExternalValue );
    return aExternalValue;
}

//------------------------------------------------------------------------------
Any ODateModel::translateExternalValueToControlValue( const Any& _rExternalValue ) const
{
    Any aControlValue;
    if ( _rExternalValue.hasValue() )
    {
        util::Date aDate;
        OSL_VERIFY( _rExternalValue >>= aDate );
        aControlValue <<= DBTypeConversion::toINT32( aDate );
    }
    return aControlValue;
}

//------------------------------------------------------------------------------
Any ODateModel::translateControlValueToValidatableValue( ) const
{
    Any aValidatableValue;
    impl_translateControlValueToUNODate( aValidatableValue );
    return aValidatableValue;
}

//------------------------------------------------------------------------------
Any ODateModel::translateDbColumnToControlValue()
{
    util::Date aDate = m_xColumn->getDate();
    if (m_xColumn->wasNull())
        m_aSaveValue.clear();
    else
        // the aggregated set expects an Int32 as value ...
        m_aSaveValue <<= DBTypeConversion::toINT32(aDate);

    return m_aSaveValue;
}

//------------------------------------------------------------------------------
Any ODateModel::getDefaultForReset() const
{
    return m_aDefault;
}

//------------------------------------------------------------------------------
void ODateModel::resetNoBroadcast()
{
    OEditBaseModel::resetNoBroadcast();
    m_aSaveValue.clear();
}

//------------------------------------------------------------------------------
Sequence< Type > ODateModel::getSupportedBindingTypes()
{
    return Sequence< Type >( &::getCppuType( static_cast< util::Date* >( NULL ) ), 1 );
}

//.........................................................................
}   // namespace frm
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
