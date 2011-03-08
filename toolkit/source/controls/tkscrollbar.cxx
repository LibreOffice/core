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
#include "toolkit/controls/tkscrollbar.hxx"
#include "toolkit/helper/property.hxx"
#include "toolkit/helper/unopropertyarrayhelper.hxx"
#include <cppuhelper/typeprovider.hxx>
#include <tools/debug.hxx>

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
    UnoControlScrollBarModel::UnoControlScrollBarModel()
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
    UnoScrollBarControl::UnoScrollBarControl()
        : maAdjustmentListeners( *this )
    {
    }

    ::rtl::OUString UnoScrollBarControl::GetComponentServiceName()
    {
        return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ScrollBar"));
    }

    // ::com::sun::star::uno::XInterface
    uno::Any UnoScrollBarControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
    {
        uno::Any aRet = ::cppu::queryInterface( rType,
                                            SAL_STATIC_CAST( awt::XAdjustmentListener*, this ),
                                            SAL_STATIC_CAST( awt::XScrollBar*, this ) );
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
