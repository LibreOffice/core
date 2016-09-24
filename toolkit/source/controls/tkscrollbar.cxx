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
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/queryinterface.hxx>

#include <toolkit/awt/vclxwindows.hxx>

#include "helper/unopropertyarrayhelper.hxx"

namespace toolkit
{


    using namespace ::com::sun::star;


    //= UnoControlScrollBarModel


    UnoControlScrollBarModel::UnoControlScrollBarModel( const uno::Reference< uno::XComponentContext >& i_factory )
        :UnoControlModel( i_factory )
    {
        UNO_CONTROL_MODEL_REGISTER_PROPERTIES( VCLXScrollBar );
    }


    OUString UnoControlScrollBarModel::getServiceName( ) throw(css::uno::RuntimeException, std::exception)
    {
        return OUString::createFromAscii( szServiceName_UnoControlScrollBarModel );
    }

    OUString UnoControlScrollBarModel::getImplementationName()
        throw (css::uno::RuntimeException, std::exception)
    {
        return OUString("stardiv.Toolkit.UnoControlScrollBarModel");
    }

    css::uno::Sequence<OUString>
    UnoControlScrollBarModel::getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception)
    {
        auto s(UnoControlModel::getSupportedServiceNames());
        s.realloc(s.getLength() + 2);
        s[s.getLength() - 2] = "com.sun.star.awt.UnoControlScrollBarModel";
        s[s.getLength() - 1] = "stardiv.vcl.controlmodel.ScrollBar";
        return s;
    }

    uno::Any UnoControlScrollBarModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
    {
        switch ( nPropId )
        {
        case BASEPROPERTY_LIVE_SCROLL:
            return uno::makeAny( false );
        case BASEPROPERTY_DEFAULTCONTROL:
            return uno::makeAny( OUString::createFromAscii( szServiceName_UnoControlScrollBar ) );

        default:
            return UnoControlModel::ImplGetDefaultValue( nPropId );
        }
    }


    ::cppu::IPropertyArrayHelper& UnoControlScrollBarModel::getInfoHelper()
    {
        static UnoPropertyArrayHelper* pHelper = nullptr;
        if ( !pHelper )
        {
            uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
            pHelper = new UnoPropertyArrayHelper( aIDs );
        }
        return *pHelper;
    }


    uno::Reference< beans::XPropertySetInfo > UnoControlScrollBarModel::getPropertySetInfo(  ) throw(uno::RuntimeException, std::exception)
    {
        static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }


    //= UnoControlScrollBarModel

    UnoScrollBarControl::UnoScrollBarControl()
        :UnoControlBase()
        ,maAdjustmentListeners( *this )
    {
    }

    OUString UnoScrollBarControl::GetComponentServiceName()
    {
        return OUString("ScrollBar");
    }

    // css::uno::XInterface
    uno::Any UnoScrollBarControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException, std::exception)
    {
        uno::Any aRet = ::cppu::queryInterface( rType,
                                            (static_cast< awt::XAdjustmentListener* >(this)),
                                            (static_cast< awt::XScrollBar* >(this)) );
        return (aRet.hasValue() ? aRet : UnoControlBase::queryAggregation( rType ));
    }

    // css::lang::XTypeProvider
    IMPL_XTYPEPROVIDER_START( UnoScrollBarControl )
        cppu::UnoType<awt::XAdjustmentListener>::get(),
        cppu::UnoType<awt::XScrollBar>::get(),
        UnoControlBase::getTypes()
    IMPL_XTYPEPROVIDER_END

    void UnoScrollBarControl::dispose() throw(uno::RuntimeException, std::exception)
    {
        lang::EventObject aEvt;
        aEvt.Source = static_cast<cppu::OWeakObject*>(this);
        maAdjustmentListeners.disposeAndClear( aEvt );
        UnoControl::dispose();
    }

    void UnoScrollBarControl::createPeer( const uno::Reference< awt::XToolkit > & rxToolkit, const uno::Reference< awt::XWindowPeer >  & rParentPeer ) throw(uno::RuntimeException, std::exception)
    {
        UnoControl::createPeer( rxToolkit, rParentPeer );

        uno::Reference < awt::XScrollBar >  xScrollBar( getPeer(), uno::UNO_QUERY );
        xScrollBar->addAdjustmentListener( this );
    }

    // css::awt::XAdjustmentListener
    void UnoScrollBarControl::adjustmentValueChanged( const css::awt::AdjustmentEvent& rEvent ) throw(css::uno::RuntimeException, std::exception)
    {
        switch ( rEvent.Type )
        {
            case css::awt::AdjustmentType_ADJUST_LINE:
            case css::awt::AdjustmentType_ADJUST_PAGE:
            case css::awt::AdjustmentType_ADJUST_ABS:
            {
                uno::Reference< awt::XScrollBar > xScrollBar( getPeer(), uno::UNO_QUERY );

                if ( xScrollBar.is() )
                {
                    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_SCROLLVALUE ), uno::Any(xScrollBar->getValue()), false );
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

    // css::awt::XScrollBar
    void UnoScrollBarControl::addAdjustmentListener( const css::uno::Reference< css::awt::XAdjustmentListener > & l ) throw(css::uno::RuntimeException, std::exception)
    {
        maAdjustmentListeners.addInterface( l );
    }

    void UnoScrollBarControl::removeAdjustmentListener( const css::uno::Reference< css::awt::XAdjustmentListener > & l ) throw(css::uno::RuntimeException, std::exception)
    {
        maAdjustmentListeners.removeInterface( l );
    }

    void UnoScrollBarControl::setValue( sal_Int32 n ) throw(css::uno::RuntimeException, std::exception)
    {
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_SCROLLVALUE ), uno::makeAny( n ), true );
    }

    void UnoScrollBarControl::setValues( sal_Int32 nValue, sal_Int32 nVisible, sal_Int32 nMax ) throw(css::uno::RuntimeException, std::exception)
    {
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_SCROLLVALUE ), uno::Any(nValue), true );
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_VISIBLESIZE ), uno::Any(nVisible), true );
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_SCROLLVALUE_MAX ), uno::Any(nMax), true );
    }

    sal_Int32 UnoScrollBarControl::getValue() throw(css::uno::RuntimeException, std::exception)
    {
        sal_Int32 n = 0;
        if ( getPeer().is() )
        {
            uno::Reference< awt::XScrollBar > xScrollBar( getPeer(), uno::UNO_QUERY );
            n = xScrollBar->getValue();
        }
        return n;
    }

    void UnoScrollBarControl::setMaximum( sal_Int32 n ) throw(css::uno::RuntimeException, std::exception)
    {
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_SCROLLVALUE_MAX ), uno::makeAny( n ), true );
    }

    sal_Int32 UnoScrollBarControl::getMaximum() throw(css::uno::RuntimeException, std::exception)
    {
        sal_Int32 n = 0;
        if ( getPeer().is() )
        {
            uno::Reference< awt::XScrollBar > xScrollBar( getPeer(), uno::UNO_QUERY );
            n = xScrollBar->getMaximum();
        }
        return n;
    }

    void UnoScrollBarControl::setLineIncrement( sal_Int32 n ) throw(css::uno::RuntimeException, std::exception)
    {
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_LINEINCREMENT ), uno::makeAny( n ), true );
    }

    sal_Int32 UnoScrollBarControl::getLineIncrement() throw(css::uno::RuntimeException, std::exception)
    {
        sal_Int32 n = 0;
        if ( getPeer().is() )
        {
            uno::Reference< awt::XScrollBar > xScrollBar( getPeer(), uno::UNO_QUERY );
            n = xScrollBar->getLineIncrement();
        }
        return n;
    }

    void UnoScrollBarControl::setBlockIncrement( sal_Int32 n ) throw(css::uno::RuntimeException, std::exception)
    {
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_BLOCKINCREMENT ), uno::makeAny( n ), true );
    }

    sal_Int32 UnoScrollBarControl::getBlockIncrement() throw(css::uno::RuntimeException, std::exception)
    {
        sal_Int32 n = 0;
        if ( getPeer().is() )
        {
            uno::Reference< awt::XScrollBar > xScrollBar( getPeer(), uno::UNO_QUERY );
            n = xScrollBar->getBlockIncrement();
        }
        return n;
    }

    void UnoScrollBarControl::setVisibleSize( sal_Int32 n ) throw(css::uno::RuntimeException, std::exception)
    {
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_VISIBLESIZE ), uno::makeAny( n ), true );
    }

    sal_Int32 UnoScrollBarControl::getVisibleSize() throw(css::uno::RuntimeException, std::exception)
    {
        sal_Int32 n = 0;
        if ( getPeer().is() )
        {
            uno::Reference< awt::XScrollBar > xScrollBar( getPeer(), uno::UNO_QUERY );
            n = xScrollBar->getVisibleSize();
        }
        return n;
    }

    void UnoScrollBarControl::setOrientation( sal_Int32 n ) throw(css::uno::RuntimeException, std::exception)
    {
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_ORIENTATION ), uno::makeAny( n ), true );
    }

    sal_Int32 UnoScrollBarControl::getOrientation() throw(css::uno::RuntimeException, std::exception)
    {
        sal_Int32 n = 0;
        if ( getPeer().is() )
        {
            uno::Reference< awt::XScrollBar > xScrollBar( getPeer(), uno::UNO_QUERY );
            n = xScrollBar->getOrientation();
        }
        return n;
    }

    OUString UnoScrollBarControl::getImplementationName()
        throw (css::uno::RuntimeException, std::exception)
    {
        return OUString("stardiv.Toolkit.UnoScrollBarControl");
    }

    css::uno::Sequence<OUString> UnoScrollBarControl::getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception)
    {
        auto s(UnoControlBase::getSupportedServiceNames());
        s.realloc(s.getLength() + 2);
        s[s.getLength() - 2] = "com.sun.star.awt.UnoControlScrollBar";
        s[s.getLength() - 1] = "stardiv.vcl.control.ScrollBar";
        return s;
    }

}  // namespace toolkit


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
stardiv_Toolkit_UnoControlScrollBarModel_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new toolkit::UnoControlScrollBarModel(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
stardiv_Toolkit_UnoScrollBarControl_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new toolkit::UnoScrollBarControl());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
