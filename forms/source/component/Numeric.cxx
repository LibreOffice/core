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
#include <comphelper/processfactory.hxx>

namespace frm
{

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

ONumericControl::ONumericControl(const Reference<XComponentContext>& _rxFactory)
    :OBoundControl(_rxFactory, VCL_CONTROL_NUMERICFIELD)
{
}


StringSequence ONumericControl::getSupportedServiceNames() throw(std::exception)
{
    StringSequence aSupported = OBoundControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_CONTROL_NUMERICFIELD;
    return aSupported;
}



InterfaceRef SAL_CALL ONumericControl_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return *(new ONumericControl( comphelper::getComponentContext(_rxFactory) ));
}


Sequence<Type> ONumericControl::_getTypes()
{
    return OBoundControl::_getTypes();
}


// ONumericModel


InterfaceRef SAL_CALL ONumericModel_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return *(new ONumericModel( comphelper::getComponentContext(_rxFactory) ));
}


Sequence<Type> ONumericModel::_getTypes()
{
    return OEditBaseModel::_getTypes();
}



ONumericModel::ONumericModel(const Reference<XComponentContext>& _rxFactory)
                :OEditBaseModel( _rxFactory, VCL_CONTROLMODEL_NUMERICFIELD, FRM_SUN_CONTROL_NUMERICFIELD, true, true )
                                    // use the old control name for compytibility reasons
{

    m_nClassId = FormComponentType::NUMERICFIELD;
    initValueProperty( PROPERTY_VALUE, PROPERTY_ID_VALUE );
}


ONumericModel::ONumericModel( const ONumericModel* _pOriginal, const Reference<XComponentContext>& _rxFactory )
    :OEditBaseModel( _pOriginal, _rxFactory )
{
}


ONumericModel::~ONumericModel()
{
}

// XCloneable

IMPLEMENT_DEFAULT_CLONING( ONumericModel )

// XServiceInfo

StringSequence ONumericModel::getSupportedServiceNames() throw(std::exception)
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


void ONumericModel::describeFixedProperties( Sequence< Property >& _rProps ) const
{
    BEGIN_DESCRIBE_PROPERTIES( 2, OEditBaseModel )
        DECL_PROP3(DEFAULT_VALUE,   double,             BOUND, MAYBEDEFAULT, MAYBEVOID);
        DECL_PROP1(TABINDEX,        sal_Int16,          BOUND);
    END_DESCRIBE_PROPERTIES();
}


OUString SAL_CALL ONumericModel::getServiceName() throw ( ::com::sun::star::uno::RuntimeException, std::exception)
{
    return OUString(FRM_COMPONENT_NUMERICFIELD);  // old (non-sun) name for compatibility !
}


bool ONumericModel::commitControlValueToDbColumn( bool /*_bPostReset*/ )
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
                return false;
            }
        }
        m_aSaveValue = aControlValue;
    }
    return true;
}


Any ONumericModel::translateDbColumnToControlValue()
{
    m_aSaveValue <<= (double)m_xColumn->getDouble();
    if ( m_xColumn->wasNull() )
        m_aSaveValue.clear();

    return m_aSaveValue;
}


Any ONumericModel::getDefaultForReset() const
{
    Any aValue;
    if (m_aDefault.getValueType().getTypeClass() == TypeClass_DOUBLE)
        aValue = m_aDefault;

    return aValue;
}


void ONumericModel::resetNoBroadcast()
{
    OEditBaseModel::resetNoBroadcast();
    m_aSaveValue.clear();
}


}   // namespace frm


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
