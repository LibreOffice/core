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

#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/listenermultiplexer.hxx>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/uno3.hxx>
#include <com/sun/star/awt/XSpinValue.hpp>


namespace toolkit
{


    //= VCLXSpinButton

    typedef ::cppu::ImplHelper1 <   css::awt::XSpinValue
                                >   VCLXSpinButton_Base;

    class VCLXSpinButton final : public VCLXWindow
                         ,public VCLXSpinButton_Base
    {
    private:
        AdjustmentListenerMultiplexer maAdjustmentListeners;

    public:
        VCLXSpinButton();

    private:
        virtual ~VCLXSpinButton( ) override;

        // XInterface
        DECLARE_XINTERFACE()

        // XTypeProvider
        DECLARE_XTYPEPROVIDER()

        // XComponent
        void dispose( ) override;

        // XSpinValue
        virtual void addAdjustmentListener( const css::uno::Reference< css::awt::XAdjustmentListener >& listener ) override;
        virtual void removeAdjustmentListener( const css::uno::Reference< css::awt::XAdjustmentListener >& listener ) override;
        virtual void setValue( sal_Int32 n ) override;
        virtual void setValues( sal_Int32 minValue, sal_Int32 maxValue, sal_Int32 currentValue ) override;
        virtual sal_Int32 getValue(  ) override;
        virtual void setMinimum( sal_Int32 minValue ) override;
        virtual void setMaximum( sal_Int32 maxValue ) override;
        virtual sal_Int32 getMinimum(  ) override;
        virtual sal_Int32 getMaximum(  ) override;
        virtual void setSpinIncrement( sal_Int32 spinIncrement ) override;
        virtual sal_Int32 getSpinIncrement(  ) override;
        virtual void setOrientation( sal_Int32 orientation ) override;
        virtual sal_Int32 getOrientation(  ) override;

        // VclWindowPeer
        virtual void setProperty( const OUString& PropertyName, const cpo::uno::Any& Value ) override;
        virtual cpo::uno::Any getProperty( const OUString& PropertyName ) override;

        // VCLXWindow
        void ProcessWindowEvent( const VclWindowEvent& _rVclWindowEvent ) override;

        VCLXSpinButton( const VCLXSpinButton& ) = delete;
        VCLXSpinButton& operator=( const VCLXSpinButton& ) = delete;
    };


} // namespacetoolkit


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
