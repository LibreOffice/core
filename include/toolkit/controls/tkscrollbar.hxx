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
#ifndef INCLUDED_TOOLKIT_CONTROLS_TKSCROLLBAR_HXX
#define INCLUDED_TOOLKIT_CONTROLS_TKSCROLLBAR_HXX

#include <toolkit/controls/unocontrolmodel.hxx>
#include <toolkit/controls/unocontrolbase.hxx>
#include <com/sun/star/awt/XScrollBar.hpp>
#include <com/sun/star/awt/XAdjustmentListener.hpp>


namespace toolkit
{


    //= UnoControlScrollBarModel

    class UnoControlScrollBarModel : public UnoControlModel
    {
    protected:
        css::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
        ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() override;

    public:
                            UnoControlScrollBarModel( const css::uno::Reference< css::uno::XComponentContext >& i_factory );
                            UnoControlScrollBarModel( const UnoControlScrollBarModel& rModel ) : UnoControlModel( rModel ) {}

        rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlScrollBarModel( *this ); }

        // css::beans::XMultiPropertySet
        css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;

        // css::io::XPersistObject
        OUString SAL_CALL getServiceName() override;

        // XServiceInfo
        OUString SAL_CALL getImplementationName() override;

        css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
    };


    //= UnoControlScrollBarModel

    class UnoScrollBarControl : public UnoControlBase,
                                public css::awt::XAdjustmentListener,
                                public css::awt::XScrollBar
    {
    private:
        AdjustmentListenerMultiplexer maAdjustmentListeners;

    public:
                                    UnoScrollBarControl();
        OUString             GetComponentServiceName() override;

        css::uno::Any  SAL_CALL queryInterface( const css::uno::Type & rType ) override { return UnoControlBase::queryInterface(rType); }
        css::uno::Any  SAL_CALL queryAggregation( const css::uno::Type & rType ) override;
        void                        SAL_CALL acquire() throw() override  { OWeakAggObject::acquire(); }
        void                        SAL_CALL release() throw() override  { OWeakAggObject::release(); }
        void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) override;
        void SAL_CALL disposing( const css::lang::EventObject& Source ) override { UnoControlBase::disposing( Source ); }
        void SAL_CALL dispose(  ) override;

        // css::lang::XTypeProvider
        css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() override;
        css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() override;

        // css::awt::XAdjustmentListener
        void SAL_CALL adjustmentValueChanged( const css::awt::AdjustmentEvent& rEvent ) override;

        // css::awt::XScrollBar
        void SAL_CALL addAdjustmentListener( const css::uno::Reference< css::awt::XAdjustmentListener >& l ) override;
        void SAL_CALL removeAdjustmentListener( const css::uno::Reference< css::awt::XAdjustmentListener >& l ) override;
        void SAL_CALL setValue( sal_Int32 n ) override;
        void SAL_CALL setValues( sal_Int32 nValue, sal_Int32 nVisible, sal_Int32 nMax ) override;
        sal_Int32 SAL_CALL getValue(  ) override;
        void SAL_CALL setMaximum( sal_Int32 n ) override;
        sal_Int32 SAL_CALL getMaximum(  ) override;
        void SAL_CALL setLineIncrement( sal_Int32 n ) override;
        sal_Int32 SAL_CALL getLineIncrement(  ) override;
        void SAL_CALL setBlockIncrement( sal_Int32 n ) override;
        sal_Int32 SAL_CALL getBlockIncrement(  ) override;
        void SAL_CALL setVisibleSize( sal_Int32 n ) override;
        sal_Int32 SAL_CALL getVisibleSize(  ) override;
        void SAL_CALL setOrientation( sal_Int32 n ) override;
        sal_Int32 SAL_CALL getOrientation(  ) override;

        // css::lang::XServiceInfo
        OUString SAL_CALL getImplementationName() override;

        css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
    };


} // namespacetoolkit


#endif // INCLUDED_TOOLKIT_CONTROLS_TKSCROLLBAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
