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
#include <toolkit/helper/servicenames.hxx>
#include <toolkit/controls/unocontrolbase.hxx>
#include <toolkit/helper/macros.hxx>
#include <com/sun/star/awt/XScrollBar.hpp>
#include <com/sun/star/awt/XAdjustmentListener.hpp>
#include <com/sun/star/awt/AdjustmentType.hpp>


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
                            UnoControlScrollBarModel( const UnoControlScrollBarModel& rModel ) : UnoControlModel( rModel ) {;}

        UnoControlModel*    Clone() const override { return new UnoControlScrollBarModel( *this ); }

        // css::beans::XMultiPropertySet
        css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;

        // css::io::XPersistObject
        OUString SAL_CALL getServiceName() throw(css::uno::RuntimeException, std::exception) override;

        // XServiceInfo
        OUString SAL_CALL getImplementationName()
            throw (css::uno::RuntimeException, std::exception) override;

        css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
            throw (css::uno::RuntimeException, std::exception) override;
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

        css::uno::Any  SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override { return UnoControlBase::queryInterface(rType); }
        css::uno::Any  SAL_CALL queryAggregation( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
        void                        SAL_CALL acquire() throw() override  { OWeakAggObject::acquire(); }
        void                        SAL_CALL release() throw() override  { OWeakAggObject::release(); }
        void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) throw(css::uno::RuntimeException, std::exception) override;
        void SAL_CALL disposing( const css::lang::EventObject& Source ) throw(css::uno::RuntimeException, std::exception) override { UnoControlBase::disposing( Source ); }
        void SAL_CALL dispose(  ) throw(css::uno::RuntimeException, std::exception) override;

        // css::lang::XTypeProvider
        css::uno::Sequence< css::uno::Type >  SAL_CALL getTypes() throw(css::uno::RuntimeException, std::exception) override;
        css::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;

        // css::awt::XAdjustmentListener
        void SAL_CALL adjustmentValueChanged( const css::awt::AdjustmentEvent& rEvent ) throw(css::uno::RuntimeException, std::exception) override;

        // css::awt::XScrollBar
        void SAL_CALL addAdjustmentListener( const css::uno::Reference< css::awt::XAdjustmentListener >& l ) throw(css::uno::RuntimeException, std::exception) override;
        void SAL_CALL removeAdjustmentListener( const css::uno::Reference< css::awt::XAdjustmentListener >& l ) throw(css::uno::RuntimeException, std::exception) override;
        void SAL_CALL setValue( sal_Int32 n ) throw(css::uno::RuntimeException, std::exception) override;
        void SAL_CALL setValues( sal_Int32 nValue, sal_Int32 nVisible, sal_Int32 nMax ) throw(css::uno::RuntimeException, std::exception) override;
        sal_Int32 SAL_CALL getValue(  ) throw(css::uno::RuntimeException, std::exception) override;
        void SAL_CALL setMaximum( sal_Int32 n ) throw(css::uno::RuntimeException, std::exception) override;
        sal_Int32 SAL_CALL getMaximum(  ) throw(css::uno::RuntimeException, std::exception) override;
        void SAL_CALL setLineIncrement( sal_Int32 n ) throw(css::uno::RuntimeException, std::exception) override;
        sal_Int32 SAL_CALL getLineIncrement(  ) throw(css::uno::RuntimeException, std::exception) override;
        void SAL_CALL setBlockIncrement( sal_Int32 n ) throw(css::uno::RuntimeException, std::exception) override;
        sal_Int32 SAL_CALL getBlockIncrement(  ) throw(css::uno::RuntimeException, std::exception) override;
        void SAL_CALL setVisibleSize( sal_Int32 n ) throw(css::uno::RuntimeException, std::exception) override;
        sal_Int32 SAL_CALL getVisibleSize(  ) throw(css::uno::RuntimeException, std::exception) override;
        void SAL_CALL setOrientation( sal_Int32 n ) throw(css::uno::RuntimeException, std::exception) override;
        sal_Int32 SAL_CALL getOrientation(  ) throw(css::uno::RuntimeException, std::exception) override;

        // css::lang::XServiceInfo
        OUString SAL_CALL getImplementationName()
            throw (css::uno::RuntimeException, std::exception) override;

        css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
            throw (css::uno::RuntimeException, std::exception) override;
    };




} // namespacetoolkit


#endif // INCLUDED_TOOLKIT_CONTROLS_TKSCROLLBAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
