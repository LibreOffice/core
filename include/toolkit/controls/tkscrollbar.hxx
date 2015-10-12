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
        ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
        ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() override;

    public:
                            UnoControlScrollBarModel( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& i_factory );
                            UnoControlScrollBarModel( const UnoControlScrollBarModel& rModel ) : UnoControlModel( rModel ) {;}

        UnoControlModel*    Clone() const override { return new UnoControlScrollBarModel( *this ); }

        // ::com::sun::star::beans::XMultiPropertySet
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

        // ::com::sun::star::io::XPersistObject
        OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

        // XServiceInfo
        OUString SAL_CALL getImplementationName()
            throw (css::uno::RuntimeException, std::exception) override;

        css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
            throw (css::uno::RuntimeException, std::exception) override;
    };


    //= UnoControlScrollBarModel

    class UnoScrollBarControl : public UnoControlBase,
                                public ::com::sun::star::awt::XAdjustmentListener,
                                public ::com::sun::star::awt::XScrollBar
    {
    private:
        AdjustmentListenerMultiplexer maAdjustmentListeners;

    public:
                                    UnoScrollBarControl();
        OUString             GetComponentServiceName() override;

        ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override { return UnoControlBase::queryInterface(rType); }
        ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        void                        SAL_CALL acquire() throw() override  { OWeakAggObject::acquire(); }
        void                        SAL_CALL release() throw() override  { OWeakAggObject::release(); }
        void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException, std::exception) override { UnoControlBase::disposing( Source ); }
        void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

        // ::com::sun::star::lang::XTypeProvider
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

        // ::com::sun::star::awt::XAdjustmentListener
        void SAL_CALL adjustmentValueChanged( const ::com::sun::star::awt::AdjustmentEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

        // ::com::sun::star::awt::XScrollBar
        void SAL_CALL addAdjustmentListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XAdjustmentListener >& l ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        void SAL_CALL removeAdjustmentListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XAdjustmentListener >& l ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        void SAL_CALL setValue( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        void SAL_CALL setValues( sal_Int32 nValue, sal_Int32 nVisible, sal_Int32 nMax ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        sal_Int32 SAL_CALL getValue(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        void SAL_CALL setMaximum( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        sal_Int32 SAL_CALL getMaximum(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        void SAL_CALL setLineIncrement( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        sal_Int32 SAL_CALL getLineIncrement(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        void SAL_CALL setBlockIncrement( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        sal_Int32 SAL_CALL getBlockIncrement(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        void SAL_CALL setVisibleSize( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        sal_Int32 SAL_CALL getVisibleSize(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        void SAL_CALL setOrientation( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        sal_Int32 SAL_CALL getOrientation(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

        // ::com::sun::star::lang::XServiceInfo
        OUString SAL_CALL getImplementationName()
            throw (css::uno::RuntimeException, std::exception) override;

        css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
            throw (css::uno::RuntimeException, std::exception) override;
    };




} // namespacetoolkit


#endif // INCLUDED_TOOLKIT_CONTROLS_TKSCROLLBAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
