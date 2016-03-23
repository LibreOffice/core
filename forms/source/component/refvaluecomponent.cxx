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

#include "refvaluecomponent.hxx"

#include <tools/diagnose_ex.h>

#include <list>


namespace frm
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::form::binding;


    //=


    OReferenceValueComponent::OReferenceValueComponent( const Reference< XComponentContext >& _rxFactory, const OUString& _rUnoControlModelTypeName, const OUString& _rDefault, bool _bSupportNoCheckRefValue )
        :OBoundControlModel( _rxFactory, _rUnoControlModelTypeName, _rDefault, false, true, true )
        ,m_eDefaultChecked( TRISTATE_FALSE )
        ,m_bSupportSecondRefValue( _bSupportNoCheckRefValue )
    {
    }


    OReferenceValueComponent::OReferenceValueComponent( const OReferenceValueComponent* _pOriginal, const Reference< XComponentContext>& _rxFactory )
        :OBoundControlModel( _pOriginal, _rxFactory )
    {
        m_sReferenceValue           = _pOriginal->m_sReferenceValue;
        m_sNoCheckReferenceValue    = _pOriginal->m_sNoCheckReferenceValue;
        m_eDefaultChecked           = _pOriginal->m_eDefaultChecked;
        m_bSupportSecondRefValue    = _pOriginal->m_bSupportSecondRefValue;

        calculateExternalValueType();
    }


    OReferenceValueComponent::~OReferenceValueComponent()
    {
    }


    void OReferenceValueComponent::setReferenceValue( const OUString& _rRefValue )
    {
        m_sReferenceValue = _rRefValue;
        calculateExternalValueType();
    }


    void SAL_CALL OReferenceValueComponent::getFastPropertyValue( Any& _rValue, sal_Int32 _nHandle ) const
    {
        switch ( _nHandle )
        {
        case PROPERTY_ID_REFVALUE:          _rValue <<= m_sReferenceValue; break;
        case PROPERTY_ID_DEFAULT_STATE:    _rValue <<= (sal_Int16)m_eDefaultChecked; break;

        case PROPERTY_ID_UNCHECKED_REFVALUE:
            OSL_ENSURE( m_bSupportSecondRefValue, "OReferenceValueComponent::getFastPropertyValue: not supported!" );
            _rValue <<= m_sNoCheckReferenceValue;
            break;

        default:
            OBoundControlModel::getFastPropertyValue( _rValue, _nHandle );
        }
    }


    void SAL_CALL OReferenceValueComponent::setFastPropertyValue_NoBroadcast( sal_Int32 _nHandle, const Any& _rValue ) throw (Exception, std::exception)
    {
        switch ( _nHandle )
        {
        case PROPERTY_ID_REFVALUE :
            OSL_VERIFY( _rValue >>= m_sReferenceValue );
            calculateExternalValueType();
            break;

        case PROPERTY_ID_UNCHECKED_REFVALUE:
            OSL_ENSURE( m_bSupportSecondRefValue, "OReferenceValueComponent::setFastPropertyValue_NoBroadcast: not supported!" );
            OSL_VERIFY( _rValue >>= m_sNoCheckReferenceValue );
            break;

        case PROPERTY_ID_DEFAULT_STATE:
        {
            sal_Int16 nDefaultChecked;
            if (!(_rValue >>= nDefaultChecked) || nDefaultChecked < 0
                || nDefaultChecked > 2)
            {
                throw css::lang::IllegalArgumentException(
                    ("DefaultState property value must be a SHORT in the range"
                     " 0--2"),
                    css::uno::Reference<css::uno::XInterface>(), -1);
            }
            m_eDefaultChecked = (ToggleState)nDefaultChecked;
            resetNoBroadcast();
        }
        break;

        default:
            OBoundControlModel::setFastPropertyValue_NoBroadcast( _nHandle, _rValue );
        }
    }


    sal_Bool SAL_CALL OReferenceValueComponent::convertFastPropertyValue( Any& _rConvertedValue, Any& _rOldValue, sal_Int32 _nHandle, const Any& _rValue ) throw (IllegalArgumentException)
    {
        bool bModified = false;
        switch ( _nHandle )
        {
        case PROPERTY_ID_REFVALUE:
            bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, m_sReferenceValue );
            break;

        case PROPERTY_ID_UNCHECKED_REFVALUE:
            OSL_ENSURE( m_bSupportSecondRefValue, "OReferenceValueComponent::convertFastPropertyValue: not supported!" );
            bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, m_sNoCheckReferenceValue );
            break;

        case PROPERTY_ID_DEFAULT_STATE:
            bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, (sal_Int16)m_eDefaultChecked );
            break;

        default:
            bModified = OBoundControlModel::convertFastPropertyValue( _rConvertedValue, _rOldValue, _nHandle, _rValue );
            break;
        }
        return bModified;
    }


    Any OReferenceValueComponent::getDefaultForReset() const
    {
        return makeAny( (sal_Int16)m_eDefaultChecked );
    }


    void OReferenceValueComponent::describeFixedProperties( Sequence< Property >& _rProps ) const
    {
        BEGIN_DESCRIBE_PROPERTIES( m_bSupportSecondRefValue ? 3 : 2, OBoundControlModel )
            DECL_PROP1( REFVALUE,       OUString,    BOUND );
            DECL_PROP1( DEFAULT_STATE, sal_Int16,          BOUND );
            if ( m_bSupportSecondRefValue )
            {
                DECL_PROP1( UNCHECKED_REFVALUE, OUString,    BOUND );
            }
        END_DESCRIBE_PROPERTIES();
    }


    Sequence< Type > OReferenceValueComponent::getSupportedBindingTypes()
    {
        ::std::list< Type > aTypes;
        aTypes.push_back( cppu::UnoType<sal_Bool>::get() );

        if ( !m_sReferenceValue.isEmpty() )
            aTypes.push_front( cppu::UnoType<OUString>::get() );
            // push_front, because this is the preferred type

        return comphelper::containerToSequence<Type>(aTypes);
    }


    Any OReferenceValueComponent::translateExternalValueToControlValue( const Any& _rExternalValue ) const
    {
        sal_Int16 nState = TRISTATE_INDET;

        bool bExternalState = false;
        OUString sExternalValue;
        if ( _rExternalValue >>= bExternalState )
        {
            nState = ::sal::static_int_cast< sal_Int16 >( bExternalState ? TRISTATE_TRUE : TRISTATE_FALSE );
        }
        else if ( _rExternalValue >>= sExternalValue )
        {
            if ( sExternalValue == m_sReferenceValue )
                nState = TRISTATE_TRUE;
            else
            {
                if ( !m_bSupportSecondRefValue || ( sExternalValue == m_sNoCheckReferenceValue ) )
                    nState = TRISTATE_FALSE;
                else
                    nState = TRISTATE_INDET;
            }
        }
        else if ( !_rExternalValue.hasValue() )
        {
            nState = TRISTATE_INDET;
        }
        else
        {
            OSL_FAIL( "OReferenceValueComponent::translateExternalValueToControlValue: unexpected value type!" );
        }

        return makeAny( nState );
    }


    Any OReferenceValueComponent::translateControlValueToExternalValue( ) const
    {
        Any aExternalValue;

        try
        {
            Any aControlValue( m_xAggregateSet->getPropertyValue( PROPERTY_STATE ) );
            sal_Int16 nControlValue = TRISTATE_INDET;
            aControlValue >>= nControlValue;

            bool bBooleanExchange = getExternalValueType().getTypeClass() == TypeClass_BOOLEAN;
            bool bStringExchange = getExternalValueType().getTypeClass() == TypeClass_STRING;
            OSL_ENSURE( bBooleanExchange || bStringExchange,
                "OReferenceValueComponent::translateControlValueToExternalValue: unexpected value exchange type!" );

            switch( nControlValue )
            {
            case TRISTATE_TRUE:
                if ( bBooleanExchange )
                {
                    aExternalValue <<= true;
                }
                else if ( bStringExchange )
                {
                    aExternalValue <<= m_sReferenceValue;
                }
                break;

            case TRISTATE_FALSE:
                if ( bBooleanExchange )
                {
                    aExternalValue <<= false;
                }
                else if ( bStringExchange )
                {
                    aExternalValue <<= (m_bSupportSecondRefValue ? m_sNoCheckReferenceValue : OUString());
                }
                break;
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "OReferenceValueComponent::translateControlValueToExternalValue: caught an exception!" );
        }

        return aExternalValue;
    }


    Any OReferenceValueComponent::translateControlValueToValidatableValue( ) const
    {
        if ( !m_xAggregateSet.is() )
            return Any();

        Any aControlValue( m_xAggregateSet->getPropertyValue( PROPERTY_STATE ) );
        sal_Int16 nControlValue = TRISTATE_INDET;
        aControlValue >>= nControlValue;

        Any aValidatableValue;
        switch ( nControlValue )
        {
        case TRISTATE_TRUE:
            aValidatableValue <<= true;
            break;
        case TRISTATE_FALSE:
            aValidatableValue <<= false;
            break;
        }
        return aValidatableValue;
    }


} // namespace frm


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
