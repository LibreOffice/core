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

#include <awt/vclxspinbutton.hxx>
#include <helper/property.hxx>
#include <com/sun/star/awt/ScrollBarOrientation.hpp>

#include <vcl/toolkit/spin.hxx>
#include <vcl/svapp.hxx>
#include "vclxwindows_internal.hxx"

namespace toolkit
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::lang;


    namespace
    {
        void lcl_modifyStyle( vcl::Window* _pWindow, WinBits _nStyleBits, bool _bShouldBePresent )
        {
            WinBits nStyle = _pWindow->GetStyle();
            if ( _bShouldBePresent )
                nStyle |= _nStyleBits;
            else
                nStyle &= ~_nStyleBits;
            _pWindow->SetStyle( nStyle );
        }
    }

    VCLXSpinButton::VCLXSpinButton()
        :maAdjustmentListeners( *this )
    {
    }


    VCLXSpinButton::~VCLXSpinButton()
    {
    }


    IMPLEMENT_FORWARD_XINTERFACE2( VCLXSpinButton, VCLXWindow, VCLXSpinButton_Base )


    IMPLEMENT_FORWARD_XTYPEPROVIDER2( VCLXSpinButton, VCLXWindow, VCLXSpinButton_Base )


    void SAL_CALL VCLXSpinButton::dispose( )
    {
        {
            SolarMutexGuard aGuard;

            EventObject aDisposeEvent;
            aDisposeEvent.Source = *this;
            maAdjustmentListeners.disposeAndClear( aDisposeEvent );
        }

        VCLXWindow::dispose();
    }


    void SAL_CALL VCLXSpinButton::addAdjustmentListener( const Reference< XAdjustmentListener >& listener )
    {
        if ( listener.is() )
            maAdjustmentListeners.addInterface( listener );
    }


    void SAL_CALL VCLXSpinButton::removeAdjustmentListener( const Reference< XAdjustmentListener >& listener )
    {
        if ( listener.is() )
            maAdjustmentListeners.removeInterface( listener );
    }

    namespace
    {
        typedef void (SpinButton::*SetSpinButtonValue) (tools::Long);
        typedef tools::Long (SpinButton::*GetSpinButtonValue) () const;


        void lcl_setSpinButtonValue(vcl::Window* _pWindow, SetSpinButtonValue _pSetter, sal_Int32 _nValue )
        {
            SolarMutexGuard aGuard;
            SpinButton* pSpinButton = static_cast< SpinButton* >( _pWindow );
            if ( pSpinButton )
                (pSpinButton->*_pSetter)( _nValue );
        }


        sal_Int32 lcl_getSpinButtonValue(const vcl::Window* _pWindow, GetSpinButtonValue _pGetter )
        {
            SolarMutexGuard aGuard;

            sal_Int32 nValue = 0;

            const SpinButton* pSpinButton = static_cast< const SpinButton* >( _pWindow );
            if ( pSpinButton )
                nValue = (pSpinButton->*_pGetter)( );
            return nValue;
        }
    }


    void SAL_CALL VCLXSpinButton::setValue( sal_Int32 n )
    {
        lcl_setSpinButtonValue( GetWindow(), &SpinButton::SetValue, n );
    }


    void SAL_CALL VCLXSpinButton::setValues( sal_Int32 minValue, sal_Int32 maxValue, sal_Int32 currentValue )
    {
        SolarMutexGuard aGuard;

        setMinimum( minValue );
        setMaximum( maxValue );
        setValue( currentValue );
    }


    sal_Int32 SAL_CALL VCLXSpinButton::getValue(  )
    {
        return lcl_getSpinButtonValue( GetWindow(), &SpinButton::GetValue );
    }


    void SAL_CALL VCLXSpinButton::setMinimum( sal_Int32 minValue )
    {
        lcl_setSpinButtonValue( GetWindow(), &SpinButton::SetRangeMin, minValue );
    }


    void SAL_CALL VCLXSpinButton::setMaximum( sal_Int32 maxValue )
    {
        lcl_setSpinButtonValue( GetWindow(), &SpinButton::SetRangeMax, maxValue );
    }


    sal_Int32 SAL_CALL VCLXSpinButton::getMinimum(  )
    {
        return lcl_getSpinButtonValue( GetWindow(), &SpinButton::GetRangeMin );
    }


    sal_Int32 SAL_CALL VCLXSpinButton::getMaximum(  )
    {
        return lcl_getSpinButtonValue( GetWindow(), &SpinButton::GetRangeMax );
    }


    void SAL_CALL VCLXSpinButton::setSpinIncrement( sal_Int32 spinIncrement )
    {
        lcl_setSpinButtonValue( GetWindow(), &SpinButton::SetValueStep, spinIncrement );
    }


    sal_Int32 SAL_CALL VCLXSpinButton::getSpinIncrement(  )
    {
        return lcl_getSpinButtonValue( GetWindow(), &SpinButton::GetValueStep );
    }


    void SAL_CALL VCLXSpinButton::setOrientation( sal_Int32 orientation )
    {
        SolarMutexGuard aGuard;

        lcl_modifyStyle( GetWindow(), WB_HSCROLL, orientation == ScrollBarOrientation::HORIZONTAL );
    }


    sal_Int32 SAL_CALL VCLXSpinButton::getOrientation(  )
    {
        return  ( 0 != ( GetWindow()->GetStyle() & WB_HSCROLL ) )
            ?   ScrollBarOrientation::HORIZONTAL
            :   ScrollBarOrientation::VERTICAL;
    }


    void VCLXSpinButton::ProcessWindowEvent( const VclWindowEvent& _rVclWindowEvent )
    {
        SolarMutexClearableGuard aGuard;
        Reference< XSpinValue > xKeepAlive( this );
        VclPtr<SpinButton> pSpinButton = GetAs<SpinButton>();
        if ( !pSpinButton )
            return;

        switch ( _rVclWindowEvent.GetId() )
        {
            case VclEventId::SpinbuttonUp:
            case VclEventId::SpinbuttonDown:
                if ( maAdjustmentListeners.getLength() )
                {
                    AdjustmentEvent aEvent;
                    aEvent.Source = *this;
                    aEvent.Value = pSpinButton->GetValue();

                    aGuard.clear();
                    maAdjustmentListeners.adjustmentValueChanged( aEvent );
                }
                break;

            default:
                xKeepAlive.clear();
                aGuard.clear();
                VCLXWindow::ProcessWindowEvent( _rVclWindowEvent );
                break;
        }
    }


    void SAL_CALL VCLXSpinButton::setProperty( const OUString& PropertyName, const Any& Value )
    {
        SolarMutexGuard aGuard;

        sal_Int32 nValue = 0;
        bool  bIsLongValue = ( Value >>= nValue );

        if ( !GetWindow() )
            return;

        sal_uInt16 nPropertyId = GetPropertyId( PropertyName );
        switch ( nPropertyId )
        {
        case BASEPROPERTY_BACKGROUNDCOLOR:
            // the default implementation of the base class doesn't work here, since our
            // interpretation for this property is slightly different
            setButtonLikeFaceColor( GetWindow(), Value);
            break;

        case BASEPROPERTY_SPINVALUE:
            if ( bIsLongValue )
                setValue( nValue );
            break;

        case BASEPROPERTY_SPINVALUE_MIN:
            if ( bIsLongValue )
                setMinimum( nValue );
            break;

        case BASEPROPERTY_SPINVALUE_MAX:
            if ( bIsLongValue )
                setMaximum( nValue );
            break;

        case BASEPROPERTY_SPININCREMENT:
            if ( bIsLongValue )
                setSpinIncrement( nValue );
            break;

        case BASEPROPERTY_ORIENTATION:
            if ( bIsLongValue )
                lcl_modifyStyle( GetWindow(), WB_HSCROLL, nValue == ScrollBarOrientation::HORIZONTAL );
            break;

        default:
            VCLXWindow::setProperty( PropertyName, Value );
        }
    }


    Any SAL_CALL VCLXSpinButton::getProperty( const OUString& PropertyName )
    {
        SolarMutexGuard aGuard;

        Any aReturn;

        if ( GetWindow() )
        {
            sal_uInt16 nPropertyId = GetPropertyId( PropertyName );
            switch ( nPropertyId )
            {
            case BASEPROPERTY_BACKGROUNDCOLOR:
                // the default implementation of the base class doesn't work here, since our
                // interpretation for this property is slightly different
                aReturn = getButtonLikeFaceColor( GetWindow() );
                break;

            case BASEPROPERTY_SPINVALUE:
                aReturn <<= getValue( );
                break;

            case BASEPROPERTY_SPINVALUE_MIN:
                aReturn <<= getMinimum( );
                break;

            case BASEPROPERTY_SPINVALUE_MAX:
                aReturn <<= getMaximum( );
                break;

            case BASEPROPERTY_SPININCREMENT:
                aReturn <<= getSpinIncrement( );
                break;

            case BASEPROPERTY_ORIENTATION:
                aReturn <<= static_cast<sal_Int32>(   ( 0 != ( GetWindow()->GetStyle() & WB_HSCROLL ) )
                    ?   ScrollBarOrientation::HORIZONTAL
                    :   ScrollBarOrientation::VERTICAL
                    );
                break;

            default:
                aReturn = VCLXWindow::getProperty( PropertyName );
            }
        }
        return aReturn;
    }


}   // namespace toolkit


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
