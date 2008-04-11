/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: refvaluecomponent.cxx,v $
 * $Revision: 1.10 $
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
#include "refvaluecomponent.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

//........................................................................
namespace frm
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::form::binding;

    //====================================================================
    //=
    //====================================================================
    //--------------------------------------------------------------------
    OReferenceValueComponent::OReferenceValueComponent( const Reference< XMultiServiceFactory>& _rxFactory, const ::rtl::OUString& _rUnoControlModelTypeName, const ::rtl::OUString& _rDefault, sal_Bool _bSupportNoCheckRefValue )
        :OBoundControlModel( _rxFactory, _rUnoControlModelTypeName, _rDefault, sal_False, sal_True, sal_True )
        ,m_eDefaultChecked( STATE_NOCHECK )
        ,m_bSupportSecondRefValue( _bSupportNoCheckRefValue )
        ,m_eValueExchangeType( eBoolean )
    {
    }

    //--------------------------------------------------------------------
    OReferenceValueComponent::OReferenceValueComponent( const OReferenceValueComponent* _pOriginal, const   Reference< XMultiServiceFactory>& _rxFactory )
        :OBoundControlModel( _pOriginal, _rxFactory )
    {
        m_sReferenceValue           = _pOriginal->m_sReferenceValue;
        m_sNoCheckReferenceValue    = _pOriginal->m_sNoCheckReferenceValue;
        m_eDefaultChecked           = _pOriginal->m_eDefaultChecked;
        m_bSupportSecondRefValue    = _pOriginal->m_bSupportSecondRefValue;

        calcValueExchangeType();
    }

    //--------------------------------------------------------------------
    OReferenceValueComponent::~OReferenceValueComponent()
    {
    }

    //--------------------------------------------------------------------
    void OReferenceValueComponent::setReferenceValue( const ::rtl::OUString& _rRefValue )
    {
        m_sReferenceValue = _rRefValue;
        calcValueExchangeType();
    }

    //--------------------------------------------------------------------
    void OReferenceValueComponent::setNoCheckReferenceValue( const ::rtl::OUString& _rNoCheckRefValue )
    {
        OSL_ENSURE( m_bSupportSecondRefValue, "OReferenceValueComponent::setNoCheckReferenceValue: no support for a second reference value!" );
        if ( m_bSupportSecondRefValue )
            m_sNoCheckReferenceValue = _rNoCheckRefValue;
    }

    //--------------------------------------------------------------------
    void SAL_CALL OReferenceValueComponent::getFastPropertyValue( Any& _rValue, sal_Int32 _nHandle ) const
    {
        switch ( _nHandle )
        {
        case PROPERTY_ID_REFVALUE:          _rValue <<= m_sReferenceValue; break;
        case PROPERTY_ID_DEFAULTCHECKED:    _rValue <<= (sal_Int16)m_eDefaultChecked; break;

        case PROPERTY_ID_UNCHECKED_REFVALUE:
            OSL_ENSURE( m_bSupportSecondRefValue, "OReferenceValueComponent::getFastPropertyValue: not supported!" );
            _rValue <<= m_sNoCheckReferenceValue;
            break;

        default:
            OBoundControlModel::getFastPropertyValue( _rValue, _nHandle );
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL OReferenceValueComponent::setFastPropertyValue_NoBroadcast( sal_Int32 _nHandle, const Any& _rValue ) throw (Exception)
    {
        switch ( _nHandle )
        {
        case PROPERTY_ID_REFVALUE :
            OSL_VERIFY( _rValue >>= m_sReferenceValue );
            calcValueExchangeType();
            break;

        case PROPERTY_ID_UNCHECKED_REFVALUE:
            OSL_ENSURE( m_bSupportSecondRefValue, "OReferenceValueComponent::setFastPropertyValue_NoBroadcast: not supported!" );
            OSL_VERIFY( _rValue >>= m_sNoCheckReferenceValue );
            break;

        case PROPERTY_ID_DEFAULTCHECKED:
        {
            sal_Int16 nDefaultChecked( (sal_Int16)STATE_NOCHECK );
            OSL_VERIFY( _rValue >>= nDefaultChecked );
            m_eDefaultChecked = (CheckState)nDefaultChecked;
            resetNoBroadcast();
        }
        break;

        default:
            OBoundControlModel::setFastPropertyValue_NoBroadcast( _nHandle, _rValue );
        }
    }

    //--------------------------------------------------------------------
    sal_Bool SAL_CALL OReferenceValueComponent::convertFastPropertyValue( Any& _rConvertedValue, Any& _rOldValue, sal_Int32 _nHandle, const Any& _rValue ) throw (IllegalArgumentException)
    {
        sal_Bool bModified = sal_False;
        switch ( _nHandle )
        {
        case PROPERTY_ID_REFVALUE:
            bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, m_sReferenceValue );
            break;

        case PROPERTY_ID_UNCHECKED_REFVALUE:
            OSL_ENSURE( m_bSupportSecondRefValue, "OReferenceValueComponent::convertFastPropertyValue: not supported!" );
            bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, m_sNoCheckReferenceValue );
            break;

        case PROPERTY_ID_DEFAULTCHECKED:
            bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, (sal_Int16)m_eDefaultChecked );
            break;

        default:
            bModified = OBoundControlModel::convertFastPropertyValue( _rConvertedValue, _rOldValue, _nHandle, _rValue );
            break;
        }
        return bModified;
    }

    //------------------------------------------------------------------------------
    Any OReferenceValueComponent::getDefaultForReset() const
    {
        return makeAny( (sal_Int16)m_eDefaultChecked );
    }

    //--------------------------------------------------------------------
    void OReferenceValueComponent::describeFixedProperties( Sequence< Property >& _rProps ) const
    {
        BEGIN_DESCRIBE_PROPERTIES( m_bSupportSecondRefValue ? 3 : 2, OBoundControlModel )
            DECL_PROP1( REFVALUE,       ::rtl::OUString,    BOUND );
            DECL_PROP1( DEFAULTCHECKED, sal_Int16,          BOUND );
            if ( m_bSupportSecondRefValue )
            {
                DECL_PROP1( UNCHECKED_REFVALUE, ::rtl::OUString,    BOUND );
            }
        END_DESCRIBE_PROPERTIES();
    }

    //-----------------------------------------------------------------------------
    void OReferenceValueComponent::calcValueExchangeType()
    {
        m_eValueExchangeType = eBoolean;
        if  (  m_sReferenceValue.getLength()
            && hasExternalValueBinding()
            && getExternalValueBinding()->supportsType( ::getCppuType( static_cast< ::rtl::OUString* >( NULL ) ) )
            )
            m_eValueExchangeType = eString;
    }

    //-----------------------------------------------------------------------------
    sal_Bool OReferenceValueComponent::approveValueBinding( const Reference< XValueBinding >& _rxBinding )
    {
        OSL_PRECOND( _rxBinding.is(), "OReferenceValueComponent::approveValueBinding: invalid binding!" );

        // only strings are accepted for simplicity
        return  _rxBinding.is()
            &&  (   _rxBinding->supportsType( ::getCppuType( static_cast< ::rtl::OUString* >( NULL ) ) )
                ||  _rxBinding->supportsType( ::getCppuType( static_cast< sal_Bool* >( NULL ) ) )
                );
    }

    //-----------------------------------------------------------------------------
    void OReferenceValueComponent::onConnectedExternalValue( )
    {
        calcValueExchangeType();
        OBoundControlModel::onConnectedExternalValue( );
    }

    //-----------------------------------------------------------------------------
    Any OReferenceValueComponent::translateExternalValueToControlValue( ) const
    {
        OSL_PRECOND( getExternalValueBinding().is(), "OReferenceValueComponent::commitControlValueToExternalBinding: no active binding!" );

        sal_Int16 nState = STATE_DONTKNOW;
        if ( getExternalValueBinding().is() )
        {
            try
            {
                switch ( m_eValueExchangeType )
                {
                case eBoolean:
                {
                    Any aExternalValue = getExternalValueBinding()->getValue( ::getCppuType( static_cast< sal_Bool* >( NULL ) ) );
                    sal_Bool bState = sal_False;
                    if ( aExternalValue >>= bState )
                        nState = sal::static_int_cast< sal_Int16 >(
                            bState ? STATE_CHECK : STATE_NOCHECK );
                }
                break;

                case eString:
                {
                    Any aExternalValue = getExternalValueBinding()->getValue( ::getCppuType( static_cast< ::rtl::OUString* >( NULL ) ) );
                    ::rtl::OUString sExternalValue;
                    if ( aExternalValue >>= sExternalValue )
                    {
                        if ( sExternalValue == m_sReferenceValue )
                            nState = STATE_CHECK;
                        else
                        {
                            if ( !m_bSupportSecondRefValue || ( sExternalValue == m_sNoCheckReferenceValue ) )
                                nState = STATE_NOCHECK;
                            else
                                nState = STATE_DONTKNOW;
                        }
                    }
                }
                break;
                }
            }
            catch( const Exception& )
            {
                OSL_ENSURE( sal_False, "OReferenceValueComponent::translateExternalValueToControlValue: caught an exception!" );
            }
        }

        return makeAny( nState );
    }

    //-----------------------------------------------------------------------------
    Any OReferenceValueComponent::translateControlValueToExternalValue( ) const
    {
        Any aExternalValue;

        try
        {
            Any aControlValue( m_xAggregateSet->getPropertyValue( PROPERTY_STATE ) );
            sal_Int16 nControlValue = STATE_DONTKNOW;
            aControlValue >>= nControlValue;

            switch( nControlValue )
            {
            case STATE_CHECK:
                if ( m_eValueExchangeType == eBoolean )
                {
                    aExternalValue <<= (sal_Bool)sal_True;
                }
                else if ( m_eValueExchangeType == eString )
                {
                    aExternalValue <<= m_sReferenceValue;
                }
                break;

            case STATE_NOCHECK:
                if ( m_eValueExchangeType == eBoolean )
                {
                    aExternalValue <<= (sal_Bool)sal_False;
                }
                else if ( m_eValueExchangeType == eString )
                {
                    aExternalValue <<= m_bSupportSecondRefValue ? m_sNoCheckReferenceValue : ::rtl::OUString();
                }
                break;
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "OReferenceValueComponent::translateControlValueToExternalValue: caught an exception!" );
        }

        return aExternalValue;
    }

    //-----------------------------------------------------------------------------
    Any OReferenceValueComponent::translateControlValueToValidatableValue( ) const
    {
        if ( !m_xAggregateSet.is() )
            return Any();

        Any aControlValue( m_xAggregateSet->getPropertyValue( PROPERTY_STATE ) );
        sal_Int16 nControlValue = STATE_DONTKNOW;
        aControlValue >>= nControlValue;

        Any aValidatableValue;
        switch ( nControlValue )
        {
        case STATE_CHECK:
            aValidatableValue <<= (sal_Bool)sal_True;
            break;
        case STATE_NOCHECK:
            aValidatableValue <<= (sal_Bool)sal_False;
            break;
        }
        return aValidatableValue;
    }

//........................................................................
} // namespace frm
//........................................................................

