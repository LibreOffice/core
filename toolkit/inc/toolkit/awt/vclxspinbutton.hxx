/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vclxspinbutton.hxx,v $
 * $Revision: 1.4 $
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
#ifndef TOOLKIT_AWT_VCLXSPINBUTTON_HXX
#define TOOLKIT_AWT_VCLXSPINBUTTON_HXX

#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/listenermultiplexer.hxx>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/uno3.hxx>
#include <com/sun/star/awt/XSpinValue.hpp>

//........................................................................
namespace toolkit
{
//........................................................................

    //====================================================================
    //= VCLXSpinButton
    //====================================================================
    typedef ::cppu::ImplHelper1 <   ::com::sun::star::awt::XSpinValue
                                >   VCLXSpinButton_Base;

    class VCLXSpinButton :public VCLXWindow
                         ,public VCLXSpinButton_Base
    {
    private:
        AdjustmentListenerMultiplexer maAdjustmentListeners;

    public:
        VCLXSpinButton();

    protected:
        ~VCLXSpinButton( );

        // XInterface
        DECLARE_XINTERFACE()

        // XTypeProvider
        DECLARE_XTYPEPROVIDER()

        // XComponent
        void SAL_CALL dispose( ) throw(::com::sun::star::uno::RuntimeException);

        // XSpinValue
        virtual void SAL_CALL addAdjustmentListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XAdjustmentListener >& listener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeAdjustmentListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XAdjustmentListener >& listener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setValue( sal_Int32 n ) throw (::com::sun::star::uno::RuntimeException);
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

        // VclWindowPeer
        virtual void SAL_CALL setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException);

        // VCLXWindow
        void ProcessWindowEvent( const VclWindowEvent& _rVclWindowEvent );

    private:
        VCLXSpinButton( const VCLXSpinButton& );            // never implemented
        VCLXSpinButton& operator=( const VCLXSpinButton& ); // never implemented
    };

//........................................................................
} // namespacetoolkit
//........................................................................

#endif // TOOLKIT_INC_TOOLKIT_AWT_VCLXSPINBUTTON_HXX

