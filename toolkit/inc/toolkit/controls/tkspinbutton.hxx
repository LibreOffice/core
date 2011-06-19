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
                            UnoSpinButtonModel( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_factory );
                            UnoSpinButtonModel( const UnoSpinButtonModel& rModel ) : UnoControlModel( rModel ) {;}

        UnoControlModel*    Clone() const { return new UnoSpinButtonModel( *this ); }

        // XMultiPropertySet
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

        // XPersistObject
        ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
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
                                    UnoSpinButtonControl( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_factory );
        ::rtl::OUString             GetComponentServiceName();

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
        ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
    };



//........................................................................
} // namespacetoolkit
//........................................................................

#endif // TOOLKIT_CONTROLS_TKSPINBUTTON_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
