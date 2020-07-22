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
#ifndef INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_PCRUNODIALOGS_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_PCRUNODIALOGS_HXX

#include <svtools/genericunodialog.hxx>
#include <com/sun/star/awt/XTabControllerModel.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <comphelper/proparrhlp.hxx>


namespace pcr
{


    //= OTabOrderDialog

    class OTabOrderDialog;
    typedef ::svt::OGenericUnoDialog                                    OTabOrderDialog_DBase;
    typedef ::comphelper::OPropertyArrayUsageHelper< OTabOrderDialog >  OTabOrderDialog_PBase;

    class OTabOrderDialog
                :public OTabOrderDialog_DBase
                ,public OTabOrderDialog_PBase
    {
    protected:
        // <properties>
        css::uno::Reference< css::awt::XTabControllerModel >
                m_xTabbingModel;
        css::uno::Reference< css::awt::XControlContainer >
                m_xControlContext;
        // </properties>

    public:
        explicit OTabOrderDialog( const css::uno::Reference< css::uno::XComponentContext >& _rxContext );
        virtual ~OTabOrderDialog() override;

        // XTypeProvider
        virtual css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override;
        virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

        // XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() override;
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;

    protected:
        // OGenericUnoDialog overridables
        virtual std::unique_ptr<weld::DialogController> createDialog(const css::uno::Reference<css::awt::XWindow>& rParent) override;
    };


} // namespacepcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_PCRUNODIALOGS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
