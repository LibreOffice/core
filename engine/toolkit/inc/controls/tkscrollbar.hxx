/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#pragma once

#include <toolkit/controls/unocontrolmodel.hxx>
#include <toolkit/controls/unocontrolbase.hxx>
#include <com/sun/star/awt/XScrollBar.hpp>
#include <com/sun/star/awt/XAdjustmentListener.hpp>


namespace toolkit
{


    //= UnoControlScrollBarModel

    class UnoControlScrollBarModel final : public UnoControlModel
    {
        cpo::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
        ::cppu::IPropertyArrayHelper& getInfoHelper() override;

    public:
                            UnoControlScrollBarModel( const css::uno::Reference< css::uno::XComponentContext >& i_factory );
                            UnoControlScrollBarModel( const UnoControlScrollBarModel& rModel ) : UnoControlModel( rModel ) {}

        rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlScrollBarModel( *this ); }

        // css::beans::XMultiPropertySet
        css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override;

        // css::io::XPersistObject
        OUString getServiceName() override;

        // XServiceInfo
        OUString getImplementationName() override;

        cpo::uno::Sequence<OUString> getSupportedServiceNames() override;
    };


    //= UnoControlScrollBarModel

    class UnoScrollBarControl final : public UnoControlBase,
                                public css::awt::XAdjustmentListener,
                                public css::awt::XScrollBar
    {
    private:
        AdjustmentListenerMultiplexer maAdjustmentListeners;

    public:
                                    UnoScrollBarControl();
        OUString             GetComponentServiceName() const override;

        cpo::uno::Any  queryInterface( const cpo::uno::Type & rType ) override { return UnoControlBase::queryInterface(rType); }
        cpo::uno::Any  queryAggregation( const cpo::uno::Type & rType ) override;
        void                        acquire() noexcept override  { OWeakAggObject::acquire(); }
        void                        release() noexcept override  { OWeakAggObject::release(); }
        void createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) override;
        void disposing( const css::lang::EventObject& Source ) override { UnoControlBase::disposing( Source ); }
        void dispose(  ) override;

        // css::lang::XTypeProvider
        cpo::uno::Sequence< cpo::uno::Type >  getTypes() override;
        cpo::uno::Sequence< sal_Int8 >                     getImplementationId() override;

        // css::awt::XAdjustmentListener
        void adjustmentValueChanged( const css::awt::AdjustmentEvent& rEvent ) override;

        // css::awt::XScrollBar
        void addAdjustmentListener( const css::uno::Reference< css::awt::XAdjustmentListener >& l ) override;
        void removeAdjustmentListener( const css::uno::Reference< css::awt::XAdjustmentListener >& l ) override;
        void setValue( sal_Int32 n ) override;
        void setValues( sal_Int32 nValue, sal_Int32 nVisible, sal_Int32 nMax ) override;
        sal_Int32 getValue(  ) override;
        void setMaximum( sal_Int32 n ) override;
        sal_Int32 getMaximum(  ) override;
        void setLineIncrement( sal_Int32 n ) override;
        sal_Int32 getLineIncrement(  ) override;
        void setBlockIncrement( sal_Int32 n ) override;
        sal_Int32 getBlockIncrement(  ) override;
        void setVisibleSize( sal_Int32 n ) override;
        sal_Int32 getVisibleSize(  ) override;
        void setOrientation( sal_Int32 n ) override;
        sal_Int32 getOrientation(  ) override;

        // css::lang::XServiceInfo
        OUString getImplementationName() override;

        cpo::uno::Sequence<OUString> getSupportedServiceNames() override;
    };


} // namespacetoolkit


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
