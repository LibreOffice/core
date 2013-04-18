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
#ifndef TOOLKIT_CONTROLS_TKSPINBUTTON_HXX
#define TOOLKIT_CONTROLS_TKSPINBUTTON_HXX

#include <toolkit/controls/unocontrolmodel.hxx>
#include <toolkit/helper/servicenames.hxx>
#include <toolkit/controls/unocontrolbase.hxx>
#include <toolkit/helper/macros.hxx>
#include <com/sun/star/awt/XSpinValue.hpp>
#include <com/sun/star/awt/XAdjustmentListener.hpp>
#include <comphelper/uno3.hxx>
#include <cppuhelper/implbase2.hxx>

//........................................................................
namespace toolkit
{
//........................................................................

    //====================================================================
    //= UnoSpinButtonModel
    //====================================================================
    class UnoSpinButtonModel : public UnoControlModel
    {
    protected:
        ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const;
        ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper();

    public:
                            UnoSpinButtonModel( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& i_factory );
                            UnoSpinButtonModel( const UnoSpinButtonModel& rModel ) : UnoControlModel( rModel ) {;}

        UnoControlModel*    Clone() const { return new UnoSpinButtonModel( *this ); }

        // XMultiPropertySet
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

        // XPersistObject
        OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
    };

    //====================================================================
    //= UnoSpinButtonControl
    //====================================================================

    typedef ::cppu::ImplHelper2 <   ::com::sun::star::awt::XAdjustmentListener
                                ,   ::com::sun::star::awt::XSpinValue
                                >   UnoSpinButtonControl_Base;

    class UnoSpinButtonControl :public UnoControlBase
                               ,public UnoSpinButtonControl_Base
    {
    private:
        AdjustmentListenerMultiplexer maAdjustmentListeners;

    public:
                                    UnoSpinButtonControl();
        OUString             GetComponentServiceName();

        DECLARE_UNO3_AGG_DEFAULTS( UnoSpinButtonControl, UnoControlBase );
        ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);

        void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException);
        void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException) { UnoControlBase::disposing( Source ); }
        void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);

        // XTypeProvider
        DECLARE_XTYPEPROVIDER()

        // XAdjustmentListener
        void SAL_CALL adjustmentValueChanged( const ::com::sun::star::awt::AdjustmentEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException);

        // XSpinValue
        virtual void SAL_CALL addAdjustmentListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XAdjustmentListener >& listener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeAdjustmentListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XAdjustmentListener >& listener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setValue( sal_Int32 value ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setValues( sal_Int32 minValue, sal_Int32 maxValue, sal_Int32 currentValue ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getValue(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setMinimum( sal_Int32 minValue ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setMaximum( sal_Int32 maxValue ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getMinimum(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getMaximum(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setSpinIncrement( sal_Int32 spinIncrement ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getSpinIncrement(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setOrientation( sal_Int32 orientation ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getOrientation(  ) throw (::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
    };



//........................................................................
} // namespacetoolkit
//........................................................................

#endif // TOOLKIT_CONTROLS_TKSPINBUTTON_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
