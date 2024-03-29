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
#include <property.hxx>
#include <services.hxx>
#include <comphelper/property.hxx>
#include <comphelper/streamsection.hxx>
#include <comphelper/basicio.hxx>
#include <rtl/math.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/form/FormComponentType.hpp>

namespace frm
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::io;


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
            if ( std::isinf( nExternalValue ) )
            {
                // set the minimum or maximum of the scroll values
                OUString sLimitPropertyName = std::signbit( nExternalValue )
                    ? _rMinValueName : _rMaxValueName;
                if ( _rxProperties.is() )
                    _rxProperties->getPropertyValue( sLimitPropertyName ) >>= nControlValue;
            }
            else
            {
                nControlValue = static_cast<sal_Int32>(::rtl::math::round( nExternalValue ));
            }
        }
        else
        {
            if ( _rxProperties.is() )
                _rxProperties->getPropertyValue( _rMinValueName ) >>= nControlValue;
        }

        return Any( nControlValue );
    }


    Any translateControlIntToExternalDoubleValue( const Any& _rControlIntValue )
    {
        Any aExternalDoubleValue;
        sal_Int32 nScrollValue = 0;
        if ( _rControlIntValue >>= nScrollValue )
            aExternalDoubleValue <<= static_cast<double>(nScrollValue);
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

    OUString SAL_CALL OScrollBarModel::getImplementationName()
    {
        return "com.sun.star.comp.forms.OScrollBarModel";
    }

        // note that we're passing OControlModel as "base class". This is because
        // OBoundControlModel, our real base class, claims to support the DataAwareControlModel
        // service, which isn't really true for us. We only derive from this class
        // to benefit from the functionality for binding to spreadsheet cells
    Sequence< OUString > SAL_CALL OScrollBarModel::getSupportedServiceNames()
    {
        Sequence< OUString > aOwnNames { FRM_SUN_COMPONENT_SCROLLBAR, BINDABLE_INTEGER_VALUE_RANGE };

        return ::comphelper::combineSequences(
            getAggregateServiceNames(),
            ::comphelper::concatSequences(
                OControlModel::getSupportedServiceNames_Static(),
                aOwnNames)
        );
    }

    css::uno::Reference< css::util::XCloneable > SAL_CALL OScrollBarModel::createClone()
{
    rtl::Reference<OScrollBarModel> pClone = new OScrollBarModel(this, getContext());
    pClone->clonedFrom(this);
    return pClone;
}


    void OScrollBarModel::describeFixedProperties( Sequence< Property >& _rProps ) const
    {
        OControlModel::describeFixedProperties( _rProps );
        sal_Int32 nOldCount = _rProps.getLength();
        _rProps.realloc( nOldCount + 3);
        css::beans::Property* pProperties = _rProps.getArray() + nOldCount;
        *pProperties++ = css::beans::Property(PROPERTY_DEFAULT_SCROLL_VALUE, PROPERTY_ID_DEFAULT_SCROLL_VALUE, cppu::UnoType<sal_Int32>::get(), css::beans::PropertyAttribute::BOUND);
        *pProperties++ = css::beans::Property(PROPERTY_TABINDEX, PROPERTY_ID_TABINDEX, cppu::UnoType<sal_Int16>::get(), css::beans::PropertyAttribute::BOUND);
        *pProperties++ = css::beans::Property(PROPERTY_CONTROLSOURCEPROPERTY, PROPERTY_ID_CONTROLSOURCEPROPERTY, cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::READONLY | css::beans::PropertyAttribute::TRANSIENT);
        DBG_ASSERT( pProperties == _rProps.getArray() + _rProps.getLength(), "<...>::describeFixedProperties/getInfoHelper: forgot to adjust the count ?");
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


    void OScrollBarModel::setFastPropertyValue_NoBroadcast( sal_Int32 _nHandle, const Any& _rValue )
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
            aReturn <<= sal_Int32(0);
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
        return Any( m_nDefaultScrollValue );
    }


    OUString SAL_CALL OScrollBarModel::getServiceName()
    {
        return FRM_SUN_COMPONENT_SCROLLBAR;
    }


    void SAL_CALL OScrollBarModel::write( const Reference< XObjectOutputStream >& _rxOutStream )
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


    void SAL_CALL OScrollBarModel::read( const Reference< XObjectInputStream>& _rxInStream )
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
            "ScrollValueMin",
            "ScrollValueMax" );
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

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_forms_OScrollBarModel_get_implementation(css::uno::XComponentContext* component,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new frm::OScrollBarModel(component));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
