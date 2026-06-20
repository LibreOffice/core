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

#include <comphelper/proparrhlp.hxx>
#include <cppuhelper/implbase.hxx>

#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <sfx2/tabdlg.hxx>
#include <svtools/genericasyncunodialog.hxx>

typedef ::cppu::ImplInheritanceHelper < ::svt::OGenericUnoAsyncDialog<SfxTabDialogController>,
                                        css::beans::XPropertyAccess, css::document::XExporter >  PDFDialog_Base;

class PDFDialog final:
    public PDFDialog_Base,
    public ::comphelper::OPropertyArrayUsageHelper< PDFDialog >
{
private:
    css::uno::Sequence< css::beans::PropertyValue >   maMediaDescriptor;
    css::uno::Sequence< css::beans::PropertyValue >   maFilterData;
    css::uno::Reference< css::lang::XComponent >     mxSrcDoc;

    // OGenericUnoDialog
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
    virtual std::unique_ptr<weld::DialogController> createDialog(const css::uno::Reference<css::awt::XWindow>& rParent) override;
    virtual void executedDialog( sal_Int16 nExecutionResult ) override;
    virtual css::uno::Reference< css::beans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() override;
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;

    // OGenericUnoAsyncDialog
    virtual std::shared_ptr<SfxTabDialogController> createAsyncDialog(const css::uno::Reference<css::awt::XWindow>& rParent) override;
    virtual void runAsync(const css::uno::Reference< css::ui::dialogs::XDialogClosedListener >& xListener) override;
    virtual void executedAsyncDialog(std::shared_ptr<SfxTabDialogController> xAsyncDialog, sal_Int32 nExecutionResult) override;

    // XPropertyAccess
    using OPropertySetHelper::getPropertyValues;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getPropertyValues(  ) override;
    using OPropertySetHelper::setPropertyValues;
    virtual void SAL_CALL setPropertyValues( const css::uno::Sequence< css::beans::PropertyValue >& aProps ) override;

    // XExporter
    virtual void SAL_CALL setSourceDocument( const css::uno::Reference< css::lang::XComponent >& xDoc ) override;

public:

    explicit    PDFDialog( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual     ~PDFDialog() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
