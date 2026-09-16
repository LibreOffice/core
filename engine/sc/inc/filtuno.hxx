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

#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>

namespace com::sun::star::io { class XInputStream; }
namespace com::sun::star::awt { class XWindow; }

class ScFilterOptionsObj final : public ::cppu::WeakImplHelper<
                            css::beans::XPropertyAccess,
                            css::ui::dialogs::XExecutableDialog,
                            css::document::XImporter,
                            css::document::XExporter,
                            css::lang::XInitialization,
                            css::lang::XServiceInfo >
{
private:
    OUString     aFileName;
    OUString     aFilterName;
    OUString     aFilterOptions;
    cpo::uno::Reference< css::io::XInputStream > xInputStream;
    cpo::uno::Reference< css::awt::XWindow > xDialogParent;
    bool         bExport;

public:
                            ScFilterOptionsObj();
    virtual                 ~ScFilterOptionsObj() override;

                            // XPropertyAccess
    virtual cpo::uno::Sequence< css::beans::PropertyValue >
                            getPropertyValues() override;
    virtual void   setPropertyValues( const cpo::uno::Sequence<
                                    css::beans::PropertyValue >& aProps ) override;

                            // XExecutableDialog
    virtual void   setTitle( const OUString& aTitle ) override;
    virtual sal_Int16 execute() override;

                            // XImporter
    virtual void   setTargetDocument( const cpo::uno::Reference< css::lang::XComponent >& xDoc ) override;

                            // XExporter
    virtual void   setSourceDocument( const cpo::uno::Reference< css::lang::XComponent >& xDoc ) override;

                            // XInitialization
    virtual void   initialize(const cpo::uno::Sequence<cpo::uno::Any>& rArguments) override;

                            // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
