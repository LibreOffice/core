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
#include <cppuhelper/implbase.hxx>
#include <comphelper/uno3.hxx>
#include <com/sun/star/awt/XSpinValue.hpp>


namespace toolkit
{


    //= VCLXSpinButton

    typedef ::cppu::ImplHelper  <   css::awt::XSpinValue
                                >   VCLXSpinButton_Base;

    class VCLXSpinButton :public VCLXWindow
                         ,public VCLXSpinButton_Base
    {
    private:
        AdjustmentListenerMultiplexer maAdjustmentListeners;

    public:
        VCLXSpinButton();

    protected:
        virtual ~VCLXSpinButton( ) override;

        // XInterface
        DECLARE_XINTERFACE()

        // XTypeProvider
        DECLARE_XTYPEPROVIDER()

        // XComponent
        void SAL_CALL dispose( ) throw(css::uno::RuntimeException, std::exception) override;

        // XSpinValue
        virtual void SAL_CALL addAdjustmentListener( const css::uno::Reference< css::awt::XAdjustmentListener >& listener ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeAdjustmentListener( const css::uno::Reference< css::awt::XAdjustmentListener >& listener ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setValue( sal_Int32 n ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setValues( sal_Int32 minValue, sal_Int32 maxValue, sal_Int32 currentValue ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getValue(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setMinimum( sal_Int32 minValue ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setMaximum( sal_Int32 maxValue ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getMinimum(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getMaximum(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setSpinIncrement( sal_Int32 spinIncrement ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getSpinIncrement(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setOrientation( sal_Int32 orientation ) throw (css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Int32 SAL_CALL getOrientation(  ) throw (css::uno::RuntimeException, std::exception) override;

        // VclWindowPeer
        virtual void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) throw(css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) throw(css::uno::RuntimeException, std::exception) override;

        // VCLXWindow
        void ProcessWindowEvent( const VclWindowEvent& _rVclWindowEvent ) override;

    private:
        VCLXSpinButton( const VCLXSpinButton& ) = delete;
        VCLXSpinButton& operator=( const VCLXSpinButton& ) = delete;
    };


} // namespacetoolkit


#endif // TOOLKIT_INC_ INCLUDED_TOOLKIT_AWT_VCLXSPINBUTTON_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
