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

#ifndef INCLUDED_FILTER_SOURCE_FLASH_SWFDIALOG_HXX
#define INCLUDED_FILTER_SOURCE_FLASH_SWFDIALOG_HXX

#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/document/XExporter.hpp>

#include <comphelper/proparrhlp.hxx>
#include <svtools/genericunodialog.hxx>

namespace vcl
{
class Window;
}

typedef cppu::ImplInheritanceHelper<::svt::OGenericUnoDialog, css::beans::XPropertyAccess,
                                    css::document::XExporter>
    SWFDialog_Base;
class SWFDialog final : public ::comphelper::OPropertyArrayUsageHelper<SWFDialog>,
                        public SWFDialog_Base
{
private:
    css::uno::Sequence<css::beans::PropertyValue> maMediaDescriptor;
    css::uno::Sequence<css::beans::PropertyValue> maFilterData;
    css::uno::Reference<css::lang::XComponent> mxSrcDoc;

    // OGenericUnoDialog
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
    virtual std::unique_ptr<weld::DialogController>
    createDialog(const css::uno::Reference<css::awt::XWindow>& rParent) override;
    virtual void executedDialog(sal_Int16 nExecutionResult) override;
    virtual css::uno::Reference<css::beans::XPropertySetInfo>
        SAL_CALL getPropertySetInfo() override;
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const override;

    // XPropertyAccess
    using ::cppu::OPropertySetHelper::getPropertyValues;
    using ::cppu::OPropertySetHelper::setPropertyValues;
    virtual css::uno::Sequence<css::beans::PropertyValue> SAL_CALL getPropertyValues() override;
    virtual void SAL_CALL
    setPropertyValues(const css::uno::Sequence<css::beans::PropertyValue>& aProps) override;

    // XExporter
    virtual void SAL_CALL
    setSourceDocument(const css::uno::Reference<css::lang::XComponent>& xDoc) override;

public:
    explicit SWFDialog(const css::uno::Reference<css::uno::XComponentContext>& rxContext);
    virtual ~SWFDialog() override;
};

#endif // INCLUDED_FILTER_SOURCE_FLASH_SWFDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
