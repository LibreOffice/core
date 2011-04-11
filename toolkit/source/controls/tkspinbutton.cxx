/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_toolkit.hxx"
#include "toolkit/controls/tkspinbutton.hxx"
#include "toolkit/helper/property.hxx"
#include "toolkit/helper/unopropertyarrayhelper.hxx"
#include <com/sun/star/awt/ScrollBarOrientation.hpp>


#include <cppuhelper/typeprovider.hxx>
#include <tools/debug.hxx>

//........................................................................
namespace toolkit
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;

    //====================================================================
    //= UnoSpinButtonModel
    //====================================================================
    //--------------------------------------------------------------------
    UnoSpinButtonModel::UnoSpinButtonModel( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_factory )
        :UnoControlModel( i_factory )
    {
        ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
        ImplRegisterProperty( BASEPROPERTY_BORDER );
        ImplRegisterProperty( BASEPROPERTY_BORDERCOLOR );
        ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
        ImplRegisterProperty( BASEPROPERTY_ENABLED );
        ImplRegisterProperty( BASEPROPERTY_ENABLEVISIBLE );
        ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
        ImplRegisterProperty( BASEPROPERTY_HELPURL );
        ImplRegisterProperty( BASEPROPERTY_ORIENTATION );
        ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
        ImplRegisterProperty( BASEPROPERTY_REPEAT );
        ImplRegisterProperty( BASEPROPERTY_REPEAT_DELAY );
        ImplRegisterProperty( BASEPROPERTY_SYMBOL_COLOR );
        ImplRegisterProperty( BASEPROPERTY_SPINVALUE );
        ImplRegisterProperty( BASEPROPERTY_SPINVALUE_MIN );
        ImplRegisterProperty( BASEPROPERTY_SPINVALUE_MAX );
        ImplRegisterProperty( BASEPROPERTY_SPININCREMENT );
        ImplRegisterProperty( BASEPROPERTY_TABSTOP );
        ImplRegisterProperty( BASEPROPERTY_WRITING_MODE );
        ImplRegisterProperty( BASEPROPERTY_CONTEXT_WRITING_MODE );
    }

    //--------------------------------------------------------------------
    ::rtl::OUString UnoSpinButtonModel::getServiceName( ) throw (RuntimeException)
    {
        return ::rtl::OUString::createFromAscii( szServiceName_UnoSpinButtonModel );
    }

    //--------------------------------------------------------------------
    Any UnoSpinButtonModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
    {
        switch ( nPropId )
        {
        case BASEPROPERTY_DEFAULTCONTROL:
            return makeAny( ::rtl::OUString::createFromAscii( szServiceName_UnoSpinButtonControl ) );

        case BASEPROPERTY_BORDER:
            return makeAny( (sal_Int16) 0 );

        case BASEPROPERTY_REPEAT:
            return makeAny( (sal_Bool)sal_True );

        default:
            return UnoControlModel::ImplGetDefaultValue( nPropId );
        }
    }

    //--------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper& UnoSpinButtonModel::getInfoHelper()
    {
        static UnoPropertyArrayHelper* pHelper = NULL;
        if ( !pHelper )
        {
            Sequence<sal_Int32> aIDs = ImplGetPropertyIds();
            pHelper = new UnoPropertyArrayHelper( aIDs );
        }
        return *pHelper;
    }

    //--------------------------------------------------------------------
    Reference< XPropertySetInfo > UnoSpinButtonModel::getPropertySetInfo(  ) throw(RuntimeException)
    {
        static Reference< XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL UnoSpinButtonModel::getImplementationName(  ) throw(RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.toolkit.UnoSpinButtonModel" ) );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL UnoSpinButtonModel::getSupportedServiceNames() throw(RuntimeException)
    {
        Sequence< ::rtl::OUString > aServices( UnoControlModel::getSupportedServiceNames() );
        aServices.realloc( aServices.getLength() + 1 );
        aServices[ aServices.getLength() - 1 ] = ::rtl::OUString::createFromAscii( szServiceName_UnoSpinButtonModel );
        return aServices;
    }

    //====================================================================
    //= UnoSpinButtonControl
    //====================================================================
    //--------------------------------------------------------------------
    UnoSpinButtonControl::UnoSpinButtonControl( const Reference< XMultiServiceFactory >& i_factory )
        :UnoControlBase( i_factory )
        ,maAdjustmentListeners( *this )
    {
    }

    //--------------------------------------------------------------------
    ::rtl::OUString UnoSpinButtonControl::GetComponentServiceName()
    {
        return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SpinButton"));
    }

    //--------------------------------------------------------------------
    Any UnoSpinButtonControl::queryAggregation( const Type & rType ) throw(RuntimeException)
    {
        Any aRet = UnoControlBase::queryAggregation( rType );
        if ( !aRet.hasValue() )
            aRet = UnoSpinButtonControl_Base::queryInterface( rType );
        return aRet;
    }

    //--------------------------------------------------------------------
    IMPLEMENT_FORWARD_XTYPEPROVIDER2( UnoSpinButtonControl, UnoControlBase, UnoSpinButtonControl_Base )

    //--------------------------------------------------------------------
    void UnoSpinButtonControl::dispose() throw(RuntimeException)
    {
        ::osl::ClearableMutexGuard aGuard( GetMutex() );
        if ( maAdjustmentListeners.getLength() )
        {
            Reference< XSpinValue > xSpinnable( getPeer(), UNO_QUERY );
            if ( xSpinnable.is() )
                xSpinnable->removeAdjustmentListener( this );

            EventObject aDisposeEvent;
            aDisposeEvent.Source = *this;

            aGuard.clear();
            maAdjustmentListeners.disposeAndClear( aDisposeEvent );
        }

        UnoControl::dispose();
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL UnoSpinButtonControl::getImplementationName(  ) throw(RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.toolkit.UnoSpinButtonControl" ) );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL UnoSpinButtonControl::getSupportedServiceNames() throw(RuntimeException)
    {
        Sequence< ::rtl::OUString > aServices( UnoControlBase::getSupportedServiceNames() );
        aServices.realloc( aServices.getLength() + 1 );
        aServices[ aServices.getLength() - 1 ] = ::rtl::OUString::createFromAscii( szServiceName_UnoSpinButtonControl );
        return aServices;
    }

    //--------------------------------------------------------------------
    void UnoSpinButtonControl::createPeer( const Reference< XToolkit > & rxToolkit, const Reference< XWindowPeer >  & rParentPeer ) throw(RuntimeException)
    {
        UnoControl::createPeer( rxToolkit, rParentPeer );

        Reference < XSpinValue > xSpinnable( getPeer(), UNO_QUERY );
        if ( xSpinnable.is() )
            xSpinnable->addAdjustmentListener( this );
    }

    //--------------------------------------------------------------------
    void UnoSpinButtonControl::adjustmentValueChanged( const AdjustmentEvent& rEvent ) throw(RuntimeException)
    {
        switch ( rEvent.Type )
        {
            case AdjustmentType_ADJUST_LINE:
            case AdjustmentType_ADJUST_PAGE:
            case AdjustmentType_ADJUST_ABS:
                ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_SPINVALUE ), makeAny( rEvent.Value ), sal_False );
                break;
            default:
                OSL_FAIL( "UnoSpinButtonControl::adjustmentValueChanged - unknown Type" );
        }

        if ( maAdjustmentListeners.getLength() )
        {
            AdjustmentEvent aEvent( rEvent );
            aEvent.Source = *this;
            maAdjustmentListeners.adjustmentValueChanged( aEvent );
        }
    }

    //--------------------------------------------------------------------
    void UnoSpinButtonControl::addAdjustmentListener( const Reference< XAdjustmentListener > & listener ) throw(RuntimeException)
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        maAdjustmentListeners.addInterface( listener );
    }

    //--------------------------------------------------------------------
    void UnoSpinButtonControl::removeAdjustmentListener( const Reference< XAdjustmentListener > & listener ) throw(RuntimeException)
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        maAdjustmentListeners.removeInterface( listener );
    }

    //--------------------------------------------------------------------
    void SAL_CALL UnoSpinButtonControl::setValue( sal_Int32 value ) throw (RuntimeException)
    {
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_SPINVALUE ), makeAny( value ), sal_True );
    }

    //--------------------------------------------------------------------
    void SAL_CALL UnoSpinButtonControl::setValues( sal_Int32 minValue, sal_Int32 maxValue, sal_Int32 currentValue ) throw (RuntimeException)
    {
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_SPINVALUE_MIN ), makeAny( minValue ), sal_True );
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_SPINVALUE_MAX ), makeAny( maxValue ), sal_True );
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_SPINVALUE ), makeAny( currentValue ), sal_True );
    }

    //--------------------------------------------------------------------
    sal_Int32 SAL_CALL UnoSpinButtonControl::getValue(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        sal_Int32 nValue = 0;

        Reference< XSpinValue > xSpinnable( getPeer(), UNO_QUERY );
        if ( xSpinnable.is() )
            nValue = xSpinnable->getValue();

        return nValue;
    }

    //--------------------------------------------------------------------
    void SAL_CALL UnoSpinButtonControl::setMinimum( sal_Int32 minValue ) throw (RuntimeException)
    {
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_SPINVALUE_MIN ), makeAny( minValue ), sal_True );
    }

    //--------------------------------------------------------------------
    void SAL_CALL UnoSpinButtonControl::setMaximum( sal_Int32 maxValue ) throw (RuntimeException)
    {
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_SPINVALUE_MAX ), makeAny( maxValue ), sal_True );
    }

    //--------------------------------------------------------------------
    sal_Int32 SAL_CALL UnoSpinButtonControl::getMinimum(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        sal_Int32 nMin = 0;

        Reference< XSpinValue > xSpinnable( getPeer(), UNO_QUERY );
        if ( xSpinnable.is() )
            nMin = xSpinnable->getMinimum();

        return nMin;
    }

    //--------------------------------------------------------------------
    sal_Int32 SAL_CALL UnoSpinButtonControl::getMaximum(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        sal_Int32 nMax = 0;

        Reference< XSpinValue > xSpinnable( getPeer(), UNO_QUERY );
        if ( xSpinnable.is() )
            nMax = xSpinnable->getMaximum();

        return nMax;
    }

    //--------------------------------------------------------------------
    void SAL_CALL UnoSpinButtonControl::setSpinIncrement( sal_Int32 spinIncrement ) throw (RuntimeException)
    {
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_SPININCREMENT ), makeAny( spinIncrement ), sal_True );
    }

    //--------------------------------------------------------------------
    sal_Int32 SAL_CALL UnoSpinButtonControl::getSpinIncrement(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        sal_Int32 nIncrement = 0;

        Reference< XSpinValue > xSpinnable( getPeer(), UNO_QUERY );
        if ( xSpinnable.is() )
            nIncrement = xSpinnable->getSpinIncrement();

        return nIncrement;
    }

    //--------------------------------------------------------------------
    void SAL_CALL UnoSpinButtonControl::setOrientation( sal_Int32 orientation ) throw (NoSupportException, RuntimeException)
    {
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_ORIENTATION ), makeAny( orientation ), sal_True );
    }

    //--------------------------------------------------------------------
    sal_Int32 SAL_CALL UnoSpinButtonControl::getOrientation(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        sal_Int32 nOrientation = ScrollBarOrientation::HORIZONTAL;

        Reference< XSpinValue > xSpinnable( getPeer(), UNO_QUERY );
        if ( xSpinnable.is() )
            nOrientation = xSpinnable->getOrientation();

        return nOrientation;
    }

//........................................................................
}  // namespace toolkit
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
