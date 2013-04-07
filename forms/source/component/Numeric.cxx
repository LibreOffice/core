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

#include "Numeric.hxx"
#include <tools/debug.hxx>

//.........................................................................
namespace frm
{
//.........................................................................
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::form::binding;

//==================================================================
// ONumericControl
//==================================================================

//------------------------------------------------------------------
ONumericControl::ONumericControl(const Reference<XMultiServiceFactory>& _rxFactory)
    :OBoundControl(_rxFactory, VCL_CONTROL_NUMERICFIELD)
{
}

//------------------------------------------------------------------------------
StringSequence ONumericControl::getSupportedServiceNames() throw()
{
    StringSequence aSupported = OBoundControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_CONTROL_NUMERICFIELD;
    return aSupported;
}


//------------------------------------------------------------------
InterfaceRef SAL_CALL ONumericControl_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return *(new ONumericControl(_rxFactory));
}

//------------------------------------------------------------------------------
Sequence<Type> ONumericControl::_getTypes()
{
    return OBoundControl::_getTypes();
}

//==================================================================
// ONumericModel
//==================================================================
//------------------------------------------------------------------
InterfaceRef SAL_CALL ONumericModel_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return *(new ONumericModel(_rxFactory));
}

//------------------------------------------------------------------------------
Sequence<Type> ONumericModel::_getTypes()
{
    return OEditBaseModel::_getTypes();
}

//------------------------------------------------------------------
DBG_NAME( ONumericModel )
//------------------------------------------------------------------
ONumericModel::ONumericModel(const Reference<XMultiServiceFactory>& _rxFactory)
                :OEditBaseModel( _rxFactory, VCL_CONTROLMODEL_NUMERICFIELD, FRM_SUN_CONTROL_NUMERICFIELD, sal_True, sal_True )
                                    // use the old control name for compytibility reasons
{
    DBG_CTOR( ONumericModel, NULL );

    m_nClassId = FormComponentType::NUMERICFIELD;
    initValueProperty( PROPERTY_VALUE, PROPERTY_ID_VALUE );
}

//------------------------------------------------------------------
ONumericModel::ONumericModel( const ONumericModel* _pOriginal, const Reference<XMultiServiceFactory>& _rxFactory )
    :OEditBaseModel( _pOriginal, _rxFactory )
{
    DBG_CTOR( ONumericModel, NULL );
}

//------------------------------------------------------------------
ONumericModel::~ONumericModel()
{
    DBG_DTOR( ONumericModel, NULL );
}

// XCloneable
//------------------------------------------------------------------------------
IMPLEMENT_DEFAULT_CLONING( ONumericModel )

// XServiceInfo
//------------------------------------------------------------------------------
StringSequence ONumericModel::getSupportedServiceNames() throw()
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

    *pStoreTo++ = FRM_SUN_COMPONENT_NUMERICFIELD;
    *pStoreTo++ = FRM_SUN_COMPONENT_DATABASE_NUMERICFIELD;
    *pStoreTo++ = BINDABLE_DATABASE_NUMERIC_FIELD;

    return aSupported;
}

//------------------------------------------------------------------------------
void ONumericModel::describeFixedProperties( Sequence< Property >& _rProps ) const
{
    BEGIN_DESCRIBE_PROPERTIES( 2, OEditBaseModel )
        DECL_PROP3(DEFAULT_VALUE,   double,             BOUND, MAYBEDEFAULT, MAYBEVOID);
        DECL_PROP1(TABINDEX,        sal_Int16,          BOUND);
    END_DESCRIBE_PROPERTIES();
}

//------------------------------------------------------------------------------
OUString SAL_CALL ONumericModel::getServiceName() throw ( ::com::sun::star::uno::RuntimeException)
{
    return FRM_COMPONENT_NUMERICFIELD;  // old (non-sun) name for compatibility !
}

//------------------------------------------------------------------------------
sal_Bool ONumericModel::commitControlValueToDbColumn( bool /*_bPostReset*/ )
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
                m_xColumnUpdate->updateDouble( getDouble( aControlValue ) );
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
Any ONumericModel::translateDbColumnToControlValue()
{
    m_aSaveValue <<= (double)m_xColumn->getDouble();
    if ( m_xColumn->wasNull() )
        m_aSaveValue.clear();

    return m_aSaveValue;
}

//------------------------------------------------------------------------------
Any ONumericModel::getDefaultForReset() const
{
    Any aValue;
    if (m_aDefault.getValueType().getTypeClass() == TypeClass_DOUBLE)
        aValue = m_aDefault;

    return aValue;
}

//------------------------------------------------------------------------------
void ONumericModel::resetNoBroadcast()
{
    OEditBaseModel::resetNoBroadcast();
    m_aSaveValue.clear();
}

//.........................................................................
}   // namespace frm
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
