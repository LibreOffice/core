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
#include "scrollbar.hxx"
#include <comphelper/property.hxx>
#include <comphelper/streamsection.hxx>
#include <comphelper/basicio.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/form/FormComponentType.hpp>
#include <property.hxx>
#include <services.hxx>


namespace frm
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::io;


    //= OSpinButtonModel

    OSpinButtonModel::OSpinButtonModel( const Reference<XComponentContext>& _rxFactory )
        :OBoundControlModel( _rxFactory, VCL_CONTROLMODEL_SPINBUTTON, VCL_CONTROL_SPINBUTTON, true, true, false )
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

    OUString SAL_CALL OSpinButtonModel::getImplementationName()
    {
        return u"com.sun.star.comp.forms.OSpinButtonModel"_ustr;
    }

        // note that we're passing OControlModel as "base class". This is because
        // OBoundControlModel, our real base class, claims to support the DataAwareControlModel
        // service, which isn't really true for us. We only derive from this class
        // to benefit from the functionality for binding to spreadsheet cells
    Sequence< OUString > SAL_CALL OSpinButtonModel::getSupportedServiceNames()
    {
        Sequence< OUString > aOwnNames { FRM_SUN_COMPONENT_SPINBUTTON, BINDABLE_INTEGER_VALUE_RANGE };

        return ::comphelper::combineSequences(
            getAggregateServiceNames(),
            ::comphelper::concatSequences(
                OControlModel::getSupportedServiceNames_Static(),
                aOwnNames
            )
        );
    }

    css::uno::Reference< css::util::XCloneable > SAL_CALL OSpinButtonModel::createClone()
{
    rtl::Reference<OSpinButtonModel> pClone = new OSpinButtonModel(this, getContext());
    pClone->clonedFrom(this);
    return pClone;
}


    void OSpinButtonModel::describeFixedProperties( Sequence< Property >& _rProps ) const
    {
        OControlModel::describeFixedProperties( _rProps );
        sal_Int32 nOldCount = _rProps.getLength();
        _rProps.realloc( nOldCount + 3);
        css::beans::Property* pProperties = _rProps.getArray() + nOldCount;
        *pProperties++ = css::beans::Property(PROPERTY_DEFAULT_SPIN_VALUE, PROPERTY_ID_DEFAULT_SPIN_VALUE, cppu::UnoType<sal_Int32>::get(), css::beans::PropertyAttribute::BOUND);
        *pProperties++ = css::beans::Property(PROPERTY_TABINDEX, PROPERTY_ID_TABINDEX, cppu::UnoType<sal_Int16>::get(), css::beans::PropertyAttribute::BOUND);
        *pProperties++ = css::beans::Property(PROPERTY_CONTROLSOURCEPROPERTY, PROPERTY_ID_CONTROLSOURCEPROPERTY, cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::READONLY | css::beans::PropertyAttribute::TRANSIENT);
        DBG_ASSERT( pProperties == _rProps.getArray() + _rProps.getLength(), "<...>::describeFixedProperties/getInfoHelper: forgot to adjust the count ?");
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


    void OSpinButtonModel::setFastPropertyValue_NoBroadcast( sal_Int32 _nHandle, const Any& _rValue )
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
    {
        bool bModified( false );
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
            aReturn <<= sal_Int32(0);
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


    bool OSpinButtonModel::commitControlValueToDbColumn( bool /*_bPostReset*/ )
    {
        OSL_FAIL( "OSpinButtonModel::commitControlValueToDbColumn: never to be called (we're not bound)!" );
        return true;
    }


    Any OSpinButtonModel::getDefaultForReset() const
    {
        return Any( m_nDefaultSpinValue );
    }


    OUString SAL_CALL OSpinButtonModel::getServiceName()
    {
        return FRM_SUN_COMPONENT_SPINBUTTON;
    }


    void SAL_CALL OSpinButtonModel::write( const Reference< XObjectOutputStream >& _rxOutStream )
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


    void SAL_CALL OSpinButtonModel::read( const Reference< XObjectInputStream>& _rxInStream )
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
            u"SpinValueMin"_ustr,
            u"SpinValueMax"_ustr );
    }


    Any OSpinButtonModel::translateControlValueToExternalValue( ) const
    {
        // by definition, the base class simply obtains the property value
        return translateControlIntToExternalDoubleValue( OBoundControlModel::translateControlValueToExternalValue() );
    }


    Sequence< Type > OSpinButtonModel::getSupportedBindingTypes()
    {
        return Sequence< Type >( &cppu::UnoType<double>::get(), 1 );
    }

}   // namespace frm

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_forms_OSpinButtonModel_get_implementation(css::uno::XComponentContext* component,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new frm::OSpinButtonModel(component));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
