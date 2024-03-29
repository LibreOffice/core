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
#include <property.hxx>

#include <comphelper/property.hxx>
#include <tools/debug.hxx>
#include <comphelper/diagnose_ex.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <vector>


namespace frm
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;


    //=


    OReferenceValueComponent::OReferenceValueComponent( const Reference< XComponentContext >& _rxFactory, const OUString& _rUnoControlModelTypeName, const OUString& _rDefault )
        :OBoundControlModel( _rxFactory, _rUnoControlModelTypeName, _rDefault, false, true, true )
        ,m_eDefaultChecked( TRISTATE_FALSE )
    {
    }


    OReferenceValueComponent::OReferenceValueComponent( const OReferenceValueComponent* _pOriginal, const Reference< XComponentContext>& _rxFactory )
        :OBoundControlModel( _pOriginal, _rxFactory )
    {
        m_sReferenceValue           = _pOriginal->m_sReferenceValue;
        m_sNoCheckReferenceValue    = _pOriginal->m_sNoCheckReferenceValue;
        m_eDefaultChecked           = _pOriginal->m_eDefaultChecked;

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
        case PROPERTY_ID_DEFAULT_STATE:    _rValue <<= static_cast<sal_Int16>(m_eDefaultChecked); break;

        case PROPERTY_ID_UNCHECKED_REFVALUE:
            _rValue <<= m_sNoCheckReferenceValue;
            break;

        default:
            OBoundControlModel::getFastPropertyValue( _rValue, _nHandle );
        }
    }


    void SAL_CALL OReferenceValueComponent::setFastPropertyValue_NoBroadcast( sal_Int32 _nHandle, const Any& _rValue )
    {
        switch ( _nHandle )
        {
        case PROPERTY_ID_REFVALUE :
            OSL_VERIFY( _rValue >>= m_sReferenceValue );
            calculateExternalValueType();
            break;

        case PROPERTY_ID_UNCHECKED_REFVALUE:
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
            m_eDefaultChecked = static_cast<ToggleState>(nDefaultChecked);
            resetNoBroadcast();
        }
        break;

        default:
            OBoundControlModel::setFastPropertyValue_NoBroadcast( _nHandle, _rValue );
        }
    }


    sal_Bool SAL_CALL OReferenceValueComponent::convertFastPropertyValue( Any& _rConvertedValue, Any& _rOldValue, sal_Int32 _nHandle, const Any& _rValue )
    {
        bool bModified = false;
        switch ( _nHandle )
        {
        case PROPERTY_ID_REFVALUE:
            bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, m_sReferenceValue );
            break;

        case PROPERTY_ID_UNCHECKED_REFVALUE:
            bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, m_sNoCheckReferenceValue );
            break;

        case PROPERTY_ID_DEFAULT_STATE:
            bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, static_cast<sal_Int16>(m_eDefaultChecked) );
            break;

        default:
            bModified = OBoundControlModel::convertFastPropertyValue( _rConvertedValue, _rOldValue, _nHandle, _rValue );
            break;
        }
        return bModified;
    }


    Any OReferenceValueComponent::getDefaultForReset() const
    {
        return Any( static_cast<sal_Int16>(m_eDefaultChecked) );
    }


    void OReferenceValueComponent::describeFixedProperties( Sequence< Property >& _rProps ) const
    {
        OBoundControlModel::describeFixedProperties( _rProps );
        sal_Int32 nOldCount = _rProps.getLength();
        _rProps.realloc( nOldCount + 3);
        css::beans::Property* pProperties = _rProps.getArray() + nOldCount;
        *pProperties++ = css::beans::Property(PROPERTY_REFVALUE, PROPERTY_ID_REFVALUE, cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::BOUND);
        *pProperties++ = css::beans::Property(PROPERTY_DEFAULT_STATE, PROPERTY_ID_DEFAULT_STATE, cppu::UnoType<sal_Int16>::get(), css::beans::PropertyAttribute::BOUND);
        *pProperties++ = css::beans::Property(PROPERTY_UNCHECKED_REFVALUE, PROPERTY_ID_UNCHECKED_REFVALUE, cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::BOUND);
        DBG_ASSERT( pProperties == _rProps.getArray() + _rProps.getLength(), "<...>::describeFixedProperties/getInfoHelper: forgot to adjust the count ?");
    }


    Sequence< Type > OReferenceValueComponent::getSupportedBindingTypes()
    {
        ::std::vector< Type > aTypes;

        if ( !m_sReferenceValue.isEmpty() )
            aTypes.push_back( cppu::UnoType<OUString>::get() );

        aTypes.push_back( cppu::UnoType<sal_Bool>::get() );

        return comphelper::containerToSequence(aTypes);
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
                if ( sExternalValue == m_sNoCheckReferenceValue )
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

        return Any( nState );
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
                    aExternalValue <<= m_sNoCheckReferenceValue;
                }
                break;
            }
        }
        catch( const Exception& )
        {
            TOOLS_WARN_EXCEPTION( "forms.component", "OReferenceValueComponent::translateControlValueToExternalValue" );
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
