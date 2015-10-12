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
#ifndef INCLUDED_TOOLKIT_AWT_VCLXSPINBUTTON_HXX
#define INCLUDED_TOOLKIT_AWT_VCLXSPINBUTTON_HXX

#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/listenermultiplexer.hxx>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/uno3.hxx>
#include <com/sun/star/awt/XSpinValue.hpp>


namespace toolkit
{



    //= VCLXSpinButton

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
        virtual ~VCLXSpinButton( );

        // XInterface
        DECLARE_XINTERFACE()

        // XTypeProvider
        DECLARE_XTYPEPROVIDER()

        // XComponent
        void SAL_CALL dispose( ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XSpinValue
        virtual void SAL_CALL addAdjustmentListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XAdjustmentListener >& listener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL removeAdjustmentListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XAdjustmentListener >& listener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setValue( sal_Int32 n ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setValues( sal_Int32 minValue, sal_Int32 maxValue, sal_Int32 currentValue ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Int32 SAL_CALL getValue(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setMinimum( sal_Int32 minValue ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setMaximum( sal_Int32 maxValue ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Int32 SAL_CALL getMinimum(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Int32 SAL_CALL getMaximum(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setSpinIncrement( sal_Int32 spinIncrement ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Int32 SAL_CALL getSpinIncrement(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setOrientation( sal_Int32 orientation ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Int32 SAL_CALL getOrientation(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // VclWindowPeer
        virtual void SAL_CALL setProperty( const OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // VCLXWindow
        void ProcessWindowEvent( const VclWindowEvent& _rVclWindowEvent ) SAL_OVERRIDE;

    private:
        VCLXSpinButton( const VCLXSpinButton& ) = delete;
        VCLXSpinButton& operator=( const VCLXSpinButton& ) = delete;
    };


} // namespacetoolkit


#endif // TOOLKIT_INC_ INCLUDED_TOOLKIT_AWT_VCLXSPINBUTTON_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
