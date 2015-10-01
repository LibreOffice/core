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

#include "scrollbar.hxx"
#include <comphelper/streamsection.hxx>
#include <comphelper/basicio.hxx>
#include <rtl/math.hxx>

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


    //= helper

    Any translateExternalDoubleToControlIntValue(
        const Any& _rExternalValue, const Reference< XPropertySet >& _rxProperties,
        const OUString& _rMinValueName, const OUString& _rMaxValueName )
    {
        OSL_ENSURE( _rxProperties.is(), "translateExternalDoubleToControlIntValue: no aggregate!?" );

        sal_Int32 nControlValue( 0 );
        double nExternalValue = 0;
        if ( _rExternalValue >>= nExternalValue )
        {
            if ( ::rtl::math::isInf( nExternalValue ) )
            {
                // set the minimum or maximum of the scroll values
                OUString sLimitPropertyName = ::rtl::math::isSignBitSet( nExternalValue )
                    ? _rMinValueName : _rMaxValueName;
                if ( _rxProperties.is() )
                    _rxProperties->getPropertyValue( sLimitPropertyName ) >>= nControlValue;
            }
            else
            {
                nControlValue = (sal_Int32)::rtl::math::round( nExternalValue );
            }
        }
        else
        {
            if ( _rxProperties.is() )
                _rxProperties->getPropertyValue( _rMinValueName ) >>= nControlValue;
        }

        return makeAny( nControlValue );
    }


    Any translateControlIntToExternalDoubleValue( const Any& _rControlIntValue )
    {
        Any aExternalDoubleValue;
        sal_Int32 nScrollValue = 0;
        if ( _rControlIntValue >>= nScrollValue )
            aExternalDoubleValue <<= (double)nScrollValue;
        else
        {
            OSL_FAIL( "translateControlIntToExternalDoubleValue: no integer scroll value!" );
            // aExternalDoubleValue is void here, which is okay for this purpose ...
        }

        return aExternalDoubleValue;
    }

    OScrollBarModel::OScrollBarModel( const Reference<XComponentContext>& _rxFactory )
        :OBoundControlModel( _rxFactory, VCL_CONTROLMODEL_SCROLLBAR, VCL_CONTROL_SCROLLBAR, true, true, false )
        ,m_nDefaultScrollValue( 0 )
    {

        m_nClassId = FormComponentType::SCROLLBAR;
        initValueProperty( PROPERTY_SCROLL_VALUE, PROPERTY_ID_SCROLL_VALUE );
    }


    OScrollBarModel::OScrollBarModel( const OScrollBarModel* _pOriginal, const Reference< XComponentContext >& _rxFactory )
        :OBoundControlModel( _pOriginal, _rxFactory )
    {
        m_nDefaultScrollValue = _pOriginal->m_nDefaultScrollValue;
    }


    OScrollBarModel::~OScrollBarModel( )
    {
    }

    OUString SAL_CALL OScrollBarModel::getImplementationName() throw ( RuntimeException, std::exception )
    {
        return OUString( "com.sun.star.comp.forms.OScrollBarModel" );
    }

        // note that we're passing OControlModel as "base class". This is because
        // OBoundControlModel, our real base class, claims to support the DataAwareControlModel
        // service, which isn't really true for us. We only derive from this class
        // to benefit from the functionality for binding to spreadsheet cells
    Sequence< OUString > SAL_CALL OScrollBarModel::getSupportedServiceNames() throw (RuntimeException, std::exception)
    {
        Sequence< OUString > aOwnNames( 2 );
        aOwnNames[ 0 ] = FRM_SUN_COMPONENT_SCROLLBAR;
        aOwnNames[ 1 ] = BINDABLE_INTEGER_VALUE_RANGE;

        return ::comphelper::combineSequences(
            getAggregateServiceNames(),
            ::comphelper::concatSequences(
                OControlModel::getSupportedServiceNames_Static(),
                aOwnNames)
        );
    }

    IMPLEMENT_DEFAULT_CLONING( OScrollBarModel )


    void SAL_CALL OScrollBarModel::disposing()
    {
        OBoundControlModel::disposing();
    }


    void OScrollBarModel::describeFixedProperties( Sequence< Property >& _rProps ) const
    {
        BEGIN_DESCRIBE_PROPERTIES( 3, OControlModel )
            DECL_PROP1( DEFAULT_SCROLL_VALUE, sal_Int32,       BOUND );
            DECL_PROP1( TABINDEX,             sal_Int16,       BOUND );
            DECL_PROP2( CONTROLSOURCEPROPERTY,OUString, READONLY, TRANSIENT );
        END_DESCRIBE_PROPERTIES();
    }


    void OScrollBarModel::getFastPropertyValue( Any& _rValue, sal_Int32 _nHandle ) const
    {
        switch ( _nHandle )
        {
            case PROPERTY_ID_DEFAULT_SCROLL_VALUE:
                _rValue <<= m_nDefaultScrollValue;
                break;

            default:
                OBoundControlModel::getFastPropertyValue( _rValue, _nHandle );
        }
    }


    void OScrollBarModel::setFastPropertyValue_NoBroadcast( sal_Int32 _nHandle, const Any& _rValue ) throw ( Exception, std::exception )
    {
        switch ( _nHandle )
        {
            case PROPERTY_ID_DEFAULT_SCROLL_VALUE:
                OSL_VERIFY( _rValue >>= m_nDefaultScrollValue );
                resetNoBroadcast();
                break;

            default:
                OBoundControlModel::setFastPropertyValue_NoBroadcast( _nHandle, _rValue );
        }
    }


    sal_Bool OScrollBarModel::convertFastPropertyValue(
                Any& _rConvertedValue, Any& _rOldValue, sal_Int32 _nHandle, const Any& _rValue )
                throw ( IllegalArgumentException )
    {
        bool bModified( false );
        switch ( _nHandle )
        {
            case PROPERTY_ID_DEFAULT_SCROLL_VALUE:
                bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, m_nDefaultScrollValue );
                break;

            default:
                bModified = OBoundControlModel::convertFastPropertyValue( _rConvertedValue, _rOldValue, _nHandle, _rValue );
                break;
        }
        return bModified;
    }


    Any OScrollBarModel::getPropertyDefaultByHandle( sal_Int32 _nHandle ) const
    {
        Any aReturn;

        switch ( _nHandle )
        {
        case PROPERTY_ID_DEFAULT_SCROLL_VALUE:
            aReturn <<= (sal_Int32)0;
            break;

        default:
            aReturn = OBoundControlModel::getPropertyDefaultByHandle( _nHandle );
            break;
        }

        return aReturn;
    }


    Any OScrollBarModel::translateDbColumnToControlValue( )
    {
        OSL_FAIL( "OScrollBarModel::commitControlValueToDbColumn: never to be called (we're not bound)!" );
        return Any();
    }


    bool OScrollBarModel::commitControlValueToDbColumn( bool /*_bPostReset*/ )
    {
        OSL_FAIL( "OScrollBarModel::commitControlValueToDbColumn: never to be called (we're not bound)!" );
        return true;
    }


    Any OScrollBarModel::getDefaultForReset() const
    {
        return makeAny( (sal_Int32)m_nDefaultScrollValue );
    }


    OUString SAL_CALL OScrollBarModel::getServiceName() throw( RuntimeException, std::exception )
    {
        return OUString(FRM_SUN_COMPONENT_SCROLLBAR);
    }


    void SAL_CALL OScrollBarModel::write( const Reference< XObjectOutputStream >& _rxOutStream )
        throw( IOException, RuntimeException, std::exception )
    {
        OBoundControlModel::write( _rxOutStream );
        ::osl::MutexGuard aGuard( m_aMutex );

        OStreamSection aSection( _rxOutStream );

        // version
        _rxOutStream->writeShort( 0x0001 );

        // properties
        _rxOutStream << m_nDefaultScrollValue;
        writeHelpTextCompatibly( _rxOutStream );
    }


    void SAL_CALL OScrollBarModel::read( const Reference< XObjectInputStream>& _rxInStream ) throw( IOException, RuntimeException, std::exception )
    {
        OBoundControlModel::read( _rxInStream );
        ::osl::MutexGuard aGuard( m_aMutex );

        // version
        {
            OStreamSection aSection( _rxInStream );

            sal_uInt16 nVersion = _rxInStream->readShort();
            if ( nVersion == 0x0001 )
            {
                _rxInStream >> m_nDefaultScrollValue;
                readHelpTextCompatibly( _rxInStream );
            }
            else
                defaultCommonProperties();

            // here, everything in the stream section which is left will be skipped
        }
    }


    Any OScrollBarModel::translateExternalValueToControlValue( const Any& _rExternalValue ) const
    {
        return translateExternalDoubleToControlIntValue( _rExternalValue, m_xAggregateSet,
            OUString( "ScrollValueMin" ),
            OUString( "ScrollValueMax" ) );
    }


    Any OScrollBarModel::translateControlValueToExternalValue( ) const
    {
        // by definition, the base class simply obtains the property value
        return translateControlIntToExternalDoubleValue( OBoundControlModel::translateControlValueToExternalValue() );
    }


    Sequence< Type > OScrollBarModel::getSupportedBindingTypes()
    {
        return Sequence< Type >( & cppu::UnoType<double>::get(), 1 );
    }

}   // namespace frm

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_comp_forms_OScrollBarModel_get_implementation(css::uno::XComponentContext* component,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new frm::OScrollBarModel(component));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
