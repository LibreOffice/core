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

#include "Currency.hxx"
#include <tools/debug.hxx>
#include <tools/string.hxx>
#include <unotools/localedatawrapper.hxx>
#include <vcl/svapp.hxx>
#include <unotools/syslocale.hxx>
#include <comphelper/processfactory.hxx>

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

//==================================================================
// OCurrencyControl
//==================================================================
//------------------------------------------------------------------
OCurrencyControl::OCurrencyControl(const Reference<XComponentContext>& _rxFactory)
    :OBoundControl(_rxFactory, VCL_CONTROL_CURRENCYFIELD)
{
}

//------------------------------------------------------------------
InterfaceRef SAL_CALL OCurrencyControl_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return *(new OCurrencyControl( comphelper::getComponentContext(_rxFactory) ));
}

//------------------------------------------------------------------------------
Sequence<Type> OCurrencyControl::_getTypes()
{
    return OBoundControl::_getTypes();
}

//------------------------------------------------------------------------------
StringSequence SAL_CALL OCurrencyControl::getSupportedServiceNames() throw()
{
    StringSequence aSupported = OBoundControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_CONTROL_CURRENCYFIELD;
    return aSupported;
}

//==================================================================
// OCurrencyModel
//==================================================================
//------------------------------------------------------------------
InterfaceRef SAL_CALL OCurrencyModel_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return *(new OCurrencyModel( comphelper::getComponentContext(_rxFactory) ));
}

//------------------------------------------------------------------------------
Sequence<Type> OCurrencyModel::_getTypes()
{
    return OEditBaseModel::_getTypes();
}

//------------------------------------------------------------------
void OCurrencyModel::implConstruct()
{
    if (m_xAggregateSet.is())
    {
        try
        {
            // get the system international information
            const SvtSysLocale aSysLocale;
            const LocaleDataWrapper& aLocaleInfo = aSysLocale.GetLocaleData();

            OUString sCurrencySymbol;
            sal_Bool bPrependCurrencySymbol;
            switch ( aLocaleInfo.getCurrPositiveFormat() )
            {
                case 0: // $1
                    sCurrencySymbol = String(aLocaleInfo.getCurrSymbol());
                    bPrependCurrencySymbol = sal_True;
                    break;
                case 1: // 1$
                    sCurrencySymbol = String(aLocaleInfo.getCurrSymbol());
                    bPrependCurrencySymbol = sal_False;
                    break;
                case 2: // $ 1
                    sCurrencySymbol = OUString(String(aLocaleInfo.getCurrSymbol())) + OUString(" ");
                    bPrependCurrencySymbol = sal_True;
                    break;
                case 3: // 1 $
                    sCurrencySymbol = OUString(" ") + OUString(String(aLocaleInfo.getCurrSymbol()));
                    bPrependCurrencySymbol = sal_False;
                    break;
            }
            if (!sCurrencySymbol.isEmpty())
            {
                m_xAggregateSet->setPropertyValue(PROPERTY_CURRENCYSYMBOL, makeAny(sCurrencySymbol));
                m_xAggregateSet->setPropertyValue(PROPERTY_CURRSYM_POSITION, makeAny(bPrependCurrencySymbol));
            }
        }
        catch(const Exception&)
        {
            OSL_FAIL( "OCurrencyModel::implConstruct: caught an exception while initializing the aggregate!" );
        }
    }
}

//------------------------------------------------------------------
DBG_NAME( OCurrencyModel )
//------------------------------------------------------------------
OCurrencyModel::OCurrencyModel(const Reference<XComponentContext>& _rxFactory)
    :OEditBaseModel( _rxFactory, VCL_CONTROLMODEL_CURRENCYFIELD, FRM_SUN_CONTROL_CURRENCYFIELD, sal_False, sal_True )
                                    // use the old control name for compytibility reasons
{
    DBG_CTOR( OCurrencyModel, NULL );

    m_nClassId = FormComponentType::CURRENCYFIELD;
    initValueProperty( PROPERTY_VALUE, PROPERTY_ID_VALUE );

    implConstruct();
}

//------------------------------------------------------------------
OCurrencyModel::OCurrencyModel( const OCurrencyModel* _pOriginal, const Reference<XComponentContext>& _rxFactory )
    :OEditBaseModel( _pOriginal, _rxFactory )
{
    DBG_CTOR( OCurrencyModel, NULL );
    implConstruct();
}

//------------------------------------------------------------------
OCurrencyModel::~OCurrencyModel()
{
    DBG_DTOR( OCurrencyModel, NULL );
}

// XCloneable
//------------------------------------------------------------------------------
IMPLEMENT_DEFAULT_CLONING( OCurrencyModel )

// XServiceInfo
//------------------------------------------------------------------------------
StringSequence SAL_CALL OCurrencyModel::getSupportedServiceNames() throw()
{
    StringSequence aSupported = OBoundControlModel::getSupportedServiceNames();

    sal_Int32 nOldLen = aSupported.getLength();
    aSupported.realloc( nOldLen + 4 );
    OUString* pStoreTo = aSupported.getArray() + nOldLen;

    *pStoreTo++ = DATA_AWARE_CONTROL_MODEL;
    *pStoreTo++ = VALIDATABLE_CONTROL_MODEL;

    *pStoreTo++ = FRM_SUN_COMPONENT_CURRENCYFIELD;
    *pStoreTo++ = FRM_SUN_COMPONENT_DATABASE_CURRENCYFIELD;

    return aSupported;
}

//------------------------------------------------------------------------------
void OCurrencyModel::describeFixedProperties( Sequence< Property >& _rProps ) const
{
    BEGIN_DESCRIBE_PROPERTIES( 2, OEditBaseModel )
        // Value auf transient setzen
//      ModifyPropertyAttributes(_rAggregateProps, PROPERTY_VALUE, PropertyAttribute::TRANSIENT, 0);

        DECL_PROP3(DEFAULT_VALUE,       double,             BOUND, MAYBEDEFAULT, MAYBEVOID);
        DECL_PROP1(TABINDEX,        sal_Int16,              BOUND);
    END_DESCRIBE_PROPERTIES();
}

//------------------------------------------------------------------------------
OUString SAL_CALL OCurrencyModel::getServiceName() throw ( ::com::sun::star::uno::RuntimeException)
{
    return OUString(FRM_COMPONENT_CURRENCYFIELD); // old (non-sun) name for compatibility !
}

//------------------------------------------------------------------------------
sal_Bool OCurrencyModel::commitControlValueToDbColumn( bool /*_bPostReset*/ )
{
    Any aControlValue( m_xAggregateFastSet->getFastPropertyValue( getValuePropertyAggHandle() ) );
    if ( !compare( aControlValue, m_aSaveValue ) )
    {
        if ( aControlValue.getValueType().getTypeClass() == TypeClass_VOID )
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
Any OCurrencyModel::translateDbColumnToControlValue()
{
    m_aSaveValue <<= m_xColumn->getDouble();
    if ( m_xColumn->wasNull() )
        m_aSaveValue.clear();
    return m_aSaveValue;
}

// XReset
//------------------------------------------------------------------------------
Any OCurrencyModel::getDefaultForReset() const
{
    Any aValue;
    if ( m_aDefault.getValueType().getTypeClass() == TypeClass_DOUBLE )
        aValue = m_aDefault;

    return aValue;
}

//------------------------------------------------------------------------------
void OCurrencyModel::resetNoBroadcast()
{
    OEditBaseModel::resetNoBroadcast();
    m_aSaveValue.clear();
}

//.........................................................................
}   // namespace frm
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
