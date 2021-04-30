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

#include <svtools/genericunodialog.hxx>
#include <comphelper/proparrhlp.hxx>
#include <componentmodule.hxx>
#include <com/sun/star/task/XJob.hpp>
#include <cppuhelper/implbase1.hxx>

namespace abp
{
    class OABSPilotUno;
    typedef ::cppu::ImplHelper1< css::task::XJob >     OABSPilotUno_JBase;
    typedef ::comphelper::OPropertyArrayUsageHelper< OABSPilotUno > OABSPilotUno_PBase;
    /// the UNO wrapper for the address book source pilot
    class OABSPilotUno
            : public svt::OGenericUnoDialog
            , public OABSPilotUno_JBase
            , public OABSPilotUno_PBase
    {
        OUString m_sDataSourceName;

    public:
        explicit OABSPilotUno(const css::uno::Reference< css::uno::XComponentContext >& _rxORB);

    private:
        // XInterface (disambiguation)
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;
        virtual void SAL_CALL acquire(  ) noexcept override;
        virtual void SAL_CALL release(  ) noexcept override;

        // XTypeProvider
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
        virtual css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override;
        virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

        // XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() override;
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;

        // XJob
        virtual css::uno::Any SAL_CALL execute( const css::uno::Sequence< css::beans::NamedValue >& lArgs ) override;

        // XInitialisation
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

        using svt::OGenericUnoDialog::execute;
        // OGenericUnoDialog overridables
        virtual std::unique_ptr<weld::DialogController> createDialog(const css::uno::Reference<css::awt::XWindow>& rParent) override;
        virtual void executedDialog(sal_Int16 _nExecutionResult) override;
    };

}   // namespace abp
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
