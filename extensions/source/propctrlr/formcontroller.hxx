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
#pragma once

#include "propcontroller.hxx"

#include <cppuhelper/propshlp.hxx>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/uno3.hxx>


namespace pcr
{


    //= FormController

    class FormController;
    typedef ::cppu::OPropertySetHelper                                  FormController_PropertyBase1;
    typedef ::comphelper::OPropertyArrayUsageHelper< FormController >   FormController_PropertyBase2;

    /** Legacy implementation of com.sun.star.form.PropertyBrowserController

        Nowadays only a wrapper around an ObjectInspector using a
        DefaultFormComponentInspectorModel.
    */
    class FormController    :public OPropertyBrowserController
                            ,public FormController_PropertyBase1
                            ,public FormController_PropertyBase2
    {
    private:
        css::uno::Reference< css::beans::XPropertySet >
                                    m_xCurrentInspectee;
        OUString                    m_sImplementationName;
        css::uno::Sequence<OUString> m_aSupportedServiceNames;
    public:
        FormController(
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext,
            const OUString& sImplementName,
            const css::uno::Sequence<OUString>& aSupportedServiceNames,
            bool _bUseFormFormComponentHandlers
        );

    protected:
        virtual ~FormController() override;

        DECLARE_XINTERFACE()
        DECLARE_XTYPEPROVIDER()

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

        // XPropertySet and friends
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;

        virtual sal_Bool SAL_CALL convertFastPropertyValue(
                css::uno::Any & rConvertedValue, css::uno::Any & rOldValue, sal_Int32 nHandle, const css::uno::Any& rValue
            ) override;
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                sal_Int32 nHandle, const css::uno::Any& rValue
            ) override;
        virtual void SAL_CALL getFastPropertyValue(
                css::uno::Any& rValue, sal_Int32 nHandle
            ) const override;
    private:
        using FormController_PropertyBase1::getFastPropertyValue;
    };


    //= DialogController

    /** Legacy implementation of com.sun.star.awt.PropertyBrowserController
    */
    class DialogController
    {
    private:
        DialogController( const DialogController& ) = delete;
        DialogController& operator=( const DialogController& ) = delete;
    };

} // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
