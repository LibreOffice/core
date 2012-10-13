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

#include "toolkit/controls/tkscrollbar.hxx"
#include "toolkit/helper/property.hxx"
#include "toolkit/helper/unopropertyarrayhelper.hxx"
#include <cppuhelper/typeprovider.hxx>

// for introspection
#include <toolkit/awt/vclxwindows.hxx>

//........................................................................
namespace toolkit
{
//........................................................................

    using namespace ::com::sun::star;

    //====================================================================
    //= UnoControlScrollBarModel
    //====================================================================
    //--------------------------------------------------------------------
    UnoControlScrollBarModel::UnoControlScrollBarModel( const uno::Reference< lang::XMultiServiceFactory >& i_factory )
        :UnoControlModel( i_factory )
    {
        UNO_CONTROL_MODEL_REGISTER_PROPERTIES( VCLXScrollBar );
    }

    //--------------------------------------------------------------------
    ::rtl::OUString UnoControlScrollBarModel::getServiceName( ) throw(::com::sun::star::uno::RuntimeException)
    {
        return ::rtl::OUString::createFromAscii( szServiceName_UnoControlScrollBarModel );
    }

    //--------------------------------------------------------------------
    uno::Any UnoControlScrollBarModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
    {
        switch ( nPropId )
        {
        case BASEPROPERTY_LIVE_SCROLL:
            return uno::makeAny( (sal_Bool)sal_False );
        case BASEPROPERTY_DEFAULTCONTROL:
            return uno::makeAny( ::rtl::OUString::createFromAscii( szServiceName_UnoControlScrollBar ) );

        default:
            return UnoControlModel::ImplGetDefaultValue( nPropId );
        }
    }

    //--------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper& UnoControlScrollBarModel::getInfoHelper()
    {
        static UnoPropertyArrayHelper* pHelper = NULL;
        if ( !pHelper )
        {
            uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
            pHelper = new UnoPropertyArrayHelper( aIDs );
        }
        return *pHelper;
    }

    //--------------------------------------------------------------------
    uno::Reference< beans::XPropertySetInfo > UnoControlScrollBarModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
    {
        static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }


    //====================================================================
    //= UnoControlScrollBarModel
    //====================================================================
    UnoScrollBarControl::UnoScrollBarControl( const uno::Reference< lang::XMultiServiceFactory >& i_factory )
        :UnoControlBase( i_factory )
        ,maAdjustmentListeners( *this )
    {
    }

    ::rtl::OUString UnoScrollBarControl::GetComponentServiceName()
    {
        return ::rtl::OUString("ScrollBar");
    }

    // ::com::sun::star::uno::XInterface
    uno::Any UnoScrollBarControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
    {
        uno::Any aRet = ::cppu::queryInterface( rType,
                                            (static_cast< awt::XAdjustmentListener* >(this)),
                                            (static_cast< awt::XScrollBar* >(this)) );
        return (aRet.hasValue() ? aRet : UnoControlBase::queryAggregation( rType ));
    }

    // ::com::sun::star::lang::XTypeProvider
    IMPL_XTYPEPROVIDER_START( UnoScrollBarControl )
        getCppuType( ( uno::Reference< awt::XAdjustmentListener>* ) NULL ),
        getCppuType( ( uno::Reference< awt::XScrollBar>* ) NULL ),
        UnoControlBase::getTypes()
    IMPL_XTYPEPROVIDER_END

    void UnoScrollBarControl::dispose() throw(uno::RuntimeException)
    {
        lang::EventObject aEvt;
        aEvt.Source = (::cppu::OWeakObject*)this;
        maAdjustmentListeners.disposeAndClear( aEvt );
        UnoControl::dispose();
    }

    void UnoScrollBarControl::createPeer( const uno::Reference< awt::XToolkit > & rxToolkit, const uno::Reference< awt::XWindowPeer >  & rParentPeer ) throw(uno::RuntimeException)
    {
        UnoControl::createPeer( rxToolkit, rParentPeer );

        uno::Reference < awt::XScrollBar >  xScrollBar( getPeer(), uno::UNO_QUERY );
        xScrollBar->addAdjustmentListener( this );
    }

    // ::com::sun::star::awt::XAdjustmentListener
    void UnoScrollBarControl::adjustmentValueChanged( const ::com::sun::star::awt::AdjustmentEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException)
    {
        switch ( rEvent.Type )
        {
            case ::com::sun::star::awt::AdjustmentType_ADJUST_LINE:
            case ::com::sun::star::awt::AdjustmentType_ADJUST_PAGE:
            case ::com::sun::star::awt::AdjustmentType_ADJUST_ABS:
            {
                uno::Reference< awt::XScrollBar > xScrollBar( getPeer(), uno::UNO_QUERY );

                if ( xScrollBar.is() )
                {
                    uno::Any aAny;
                    aAny <<= xScrollBar->getValue();
                    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_SCROLLVALUE ), aAny, sal_False );
                }
            }
            break;
            default:
            {
                OSL_FAIL( "UnoScrollBarControl::adjustmentValueChanged - unknown Type" );

            }
        }

        if ( maAdjustmentListeners.getLength() )
            maAdjustmentListeners.adjustmentValueChanged( rEvent );
    }

    // ::com::sun::star::awt::XScrollBar
    void UnoScrollBarControl::addAdjustmentListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XAdjustmentListener > & l ) throw(::com::sun::star::uno::RuntimeException)
    {
        maAdjustmentListeners.addInterface( l );
    }

    void UnoScrollBarControl::removeAdjustmentListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XAdjustmentListener > & l ) throw(::com::sun::star::uno::RuntimeException)
    {
        maAdjustmentListeners.removeInterface( l );
    }

    void UnoScrollBarControl::setValue( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException)
    {
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_SCROLLVALUE ), uno::makeAny( n ), sal_True );
    }

    void UnoScrollBarControl::setValues( sal_Int32 nValue, sal_Int32 nVisible, sal_Int32 nMax ) throw(::com::sun::star::uno::RuntimeException)
    {
        uno::Any aAny;
        aAny <<= nValue;
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_SCROLLVALUE ), aAny, sal_True );
        aAny <<= nVisible;
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_VISIBLESIZE ), aAny, sal_True );
        aAny <<= nMax;
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_SCROLLVALUE_MAX ), aAny, sal_True );
    }

    sal_Int32 UnoScrollBarControl::getValue() throw(::com::sun::star::uno::RuntimeException)
    {
        sal_Int32 n = 0;
        if ( getPeer().is() )
        {
            uno::Reference< awt::XScrollBar > xScrollBar( getPeer(), uno::UNO_QUERY );
            n = xScrollBar->getValue();
        }
        return n;
    }

    void UnoScrollBarControl::setMaximum( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException)
    {
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_SCROLLVALUE_MAX ), uno::makeAny( n ), sal_True );
    }

    sal_Int32 UnoScrollBarControl::getMaximum() throw(::com::sun::star::uno::RuntimeException)
    {
        sal_Int32 n = 0;
        if ( getPeer().is() )
        {
            uno::Reference< awt::XScrollBar > xScrollBar( getPeer(), uno::UNO_QUERY );
            n = xScrollBar->getMaximum();
        }
        return n;
    }

    void UnoScrollBarControl::setLineIncrement( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException)
    {
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_LINEINCREMENT ), uno::makeAny( n ), sal_True );
    }

    sal_Int32 UnoScrollBarControl::getLineIncrement() throw(::com::sun::star::uno::RuntimeException)
    {
        sal_Int32 n = 0;
        if ( getPeer().is() )
        {
            uno::Reference< awt::XScrollBar > xScrollBar( getPeer(), uno::UNO_QUERY );
            n = xScrollBar->getLineIncrement();
        }
        return n;
    }

    void UnoScrollBarControl::setBlockIncrement( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException)
    {
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_BLOCKINCREMENT ), uno::makeAny( n ), sal_True );
    }

    sal_Int32 UnoScrollBarControl::getBlockIncrement() throw(::com::sun::star::uno::RuntimeException)
    {
        sal_Int32 n = 0;
        if ( getPeer().is() )
        {
            uno::Reference< awt::XScrollBar > xScrollBar( getPeer(), uno::UNO_QUERY );
            n = xScrollBar->getBlockIncrement();
        }
        return n;
    }

    void UnoScrollBarControl::setVisibleSize( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException)
    {
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_VISIBLESIZE ), uno::makeAny( n ), sal_True );
    }

    sal_Int32 UnoScrollBarControl::getVisibleSize() throw(::com::sun::star::uno::RuntimeException)
    {
        sal_Int32 n = 0;
        if ( getPeer().is() )
        {
            uno::Reference< awt::XScrollBar > xScrollBar( getPeer(), uno::UNO_QUERY );
            n = xScrollBar->getVisibleSize();
        }
        return n;
    }

    void UnoScrollBarControl::setOrientation( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException)
    {
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_ORIENTATION ), uno::makeAny( n ), sal_True );
    }

    sal_Int32 UnoScrollBarControl::getOrientation() throw(::com::sun::star::uno::RuntimeException)
    {
        sal_Int32 n = 0;
        if ( getPeer().is() )
        {
            uno::Reference< awt::XScrollBar > xScrollBar( getPeer(), uno::UNO_QUERY );
            n = xScrollBar->getOrientation();
        }
        return n;
    }



//........................................................................
}  // namespace toolkit
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
