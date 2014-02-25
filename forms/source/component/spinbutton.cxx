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

#include "spinbutton.hxx"
#include <comphelper/streamsection.hxx>
#include <comphelper/basicio.hxx>


extern "C" void SAL_CALL createRegistryInfo_OSpinButtonModel()
{
    static ::frm::OMultiInstanceAutoRegistration< ::frm::OSpinButtonModel >   aRegisterModel;
}


namespace frm
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::io;
    using namespace ::com::sun::star::form::binding;


    //= OSpinButtonModel

    // implemented elsewhere
    Any translateExternalDoubleToControlIntValue(
        const Any& _rExternalValue, const Reference< XPropertySet >& _rxProperties,
        const OUString& _rMinValueName, const OUString& _rMaxValueName );
    Any translateControlIntToExternalDoubleValue( const Any& _rControlIntValue );


    //= OSpinButtonModel



    OSpinButtonModel::OSpinButtonModel( const Reference<XComponentContext>& _rxFactory )
        :OBoundControlModel( _rxFactory, VCL_CONTROLMODEL_SPINBUTTON, VCL_CONTROL_SPINBUTTON, sal_True, sal_True, sal_False )
        ,m_nDefaultSpinValue( 0 )
    {

        m_nClassId = FormComponentType::SPINBUTTON;
        initValueProperty( PROPERTY_SPIN_VALUE, PROPERTY_ID_SPIN_VALUE );
    }


    OSpinButtonModel::OSpinButtonModel( const OSpinButtonModel* _pOriginal, const Reference< XComponentContext >& _rxFactory )
        :OBoundControlModel( _pOriginal, _rxFactory )
    {
        m_nDefaultSpinValue = _pOriginal->m_nDefaultSpinValue;
    }


    OSpinButtonModel::~OSpinButtonModel( )
    {
    }


    IMPLEMENT_SERVICE_REGISTRATION_2( OSpinButtonModel, OControlModel, FRM_SUN_COMPONENT_SPINBUTTON, BINDABLE_INTEGER_VALUE_RANGE )
        // note that we're passing OControlModel as "base class". This is because
        // OBoundControlModel, our real base class, claims to support the DataAwareControlModel
        // service, which isn't really true for us. We only derive from this class
        // to benefit from the functionality for binding to spreadsheet cells


    IMPLEMENT_DEFAULT_CLONING( OSpinButtonModel )


    void SAL_CALL OSpinButtonModel::disposing()
    {
        OBoundControlModel::disposing();
    }


    void OSpinButtonModel::describeFixedProperties( Sequence< Property >& _rProps ) const
    {
        BEGIN_DESCRIBE_PROPERTIES( 3, OControlModel )
            DECL_PROP1( DEFAULT_SPIN_VALUE,   sal_Int32,       BOUND );
            DECL_PROP1( TABINDEX,             sal_Int16,       BOUND );
            DECL_PROP2( CONTROLSOURCEPROPERTY,OUString, READONLY, TRANSIENT );
        END_DESCRIBE_PROPERTIES();
    }


    void OSpinButtonModel::getFastPropertyValue( Any& _rValue, sal_Int32 _nHandle ) const
    {
        switch ( _nHandle )
        {
            case PROPERTY_ID_DEFAULT_SPIN_VALUE:
                _rValue <<= m_nDefaultSpinValue;
                break;

            default:
                OBoundControlModel::getFastPropertyValue( _rValue, _nHandle );
        }
    }


    void OSpinButtonModel::setFastPropertyValue_NoBroadcast( sal_Int32 _nHandle, const Any& _rValue ) throw ( Exception, std::exception )
    {
        switch ( _nHandle )
        {
            case PROPERTY_ID_DEFAULT_SPIN_VALUE:
                OSL_VERIFY( _rValue >>= m_nDefaultSpinValue );
                resetNoBroadcast();
                break;

            default:
                OBoundControlModel::setFastPropertyValue_NoBroadcast( _nHandle, _rValue );
        }
    }


    sal_Bool OSpinButtonModel::convertFastPropertyValue(
                Any& _rConvertedValue, Any& _rOldValue, sal_Int32 _nHandle, const Any& _rValue )
                throw ( IllegalArgumentException )
    {
        sal_Bool bModified( sal_False );
        switch ( _nHandle )
        {
            case PROPERTY_ID_DEFAULT_SPIN_VALUE:
                bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, m_nDefaultSpinValue );
                break;

            default:
                bModified = OBoundControlModel::convertFastPropertyValue( _rConvertedValue, _rOldValue, _nHandle, _rValue );
                break;
        }
        return bModified;
    }


    Any OSpinButtonModel::getPropertyDefaultByHandle( sal_Int32 _nHandle ) const
    {
        Any aReturn;

        switch ( _nHandle )
        {
        case PROPERTY_ID_DEFAULT_SPIN_VALUE:
            aReturn <<= (sal_Int32)0;
            break;

        default:
            aReturn = OBoundControlModel::getPropertyDefaultByHandle( _nHandle );
            break;
        }

        return aReturn;
    }


    Any OSpinButtonModel::translateDbColumnToControlValue( )
    {
        OSL_FAIL( "OSpinButtonModel::commitControlValueToDbColumn: never to be called (we're not bound)!" );
        return Any();
    }


    sal_Bool OSpinButtonModel::commitControlValueToDbColumn( bool /*_bPostReset*/ )
    {
        OSL_FAIL( "OSpinButtonModel::commitControlValueToDbColumn: never to be called (we're not bound)!" );
        return sal_True;
    }


    Any OSpinButtonModel::getDefaultForReset() const
    {
        return makeAny( (sal_Int32)m_nDefaultSpinValue );
    }


    OUString SAL_CALL OSpinButtonModel::getServiceName() throw( RuntimeException, std::exception )
    {
        return OUString(FRM_SUN_COMPONENT_SPINBUTTON);
    }


    void SAL_CALL OSpinButtonModel::write( const Reference< XObjectOutputStream >& _rxOutStream )
        throw( IOException, RuntimeException, std::exception )
    {
        OBoundControlModel::write( _rxOutStream );
        ::osl::MutexGuard aGuard( m_aMutex );

        OStreamSection aSection( _rxOutStream );

        // version
        _rxOutStream->writeShort( 0x0001 );

        // properties
        _rxOutStream << m_nDefaultSpinValue;
        writeHelpTextCompatibly( _rxOutStream );
    }


    void SAL_CALL OSpinButtonModel::read( const Reference< XObjectInputStream>& _rxInStream ) throw( IOException, RuntimeException, std::exception )
    {
        OBoundControlModel::read( _rxInStream );
        ::osl::MutexGuard aGuard( m_aMutex );

        // version
        {
            OStreamSection aSection( _rxInStream );

            sal_uInt16 nVersion = _rxInStream->readShort();
            if ( nVersion == 0x0001 )
            {
                _rxInStream >> m_nDefaultSpinValue;
                readHelpTextCompatibly( _rxInStream );
            }
            else
                defaultCommonProperties();

            // here, everything in the stream section which is left will be skipped
        }
    }


    Any OSpinButtonModel::translateExternalValueToControlValue( const Any& _rExternalValue ) const
    {
        return translateExternalDoubleToControlIntValue( _rExternalValue, m_xAggregateSet,
            OUString( "SpinValueMin" ),
            OUString( "SpinValueMax" ) );
    }


    Any OSpinButtonModel::translateControlValueToExternalValue( ) const
    {
        // by definition, the base class simply obtains the property value
        return translateControlIntToExternalDoubleValue( OBoundControlModel::translateControlValueToExternalValue() );
    }


    Sequence< Type > OSpinButtonModel::getSupportedBindingTypes()
    {
        return Sequence< Type >( &::getCppuType( static_cast< double* >( NULL ) ), 1 );
    }


}   // namespace frm


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
