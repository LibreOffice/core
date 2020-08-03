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

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>


namespace dp_gui {

class LicenseDialog
    : public ::cppu::WeakImplHelper<css::ui::dialogs::XExecutableDialog, css::lang::XServiceInfo>
{
    css::uno::Reference<css::awt::XWindow> /* const */ m_parent;
    OUString m_sExtensionName;
    OUString /* const */ m_sLicenseText;

    sal_Int16 solar_execute();

public:
    LicenseDialog( css::uno::Sequence<css::uno::Any> const & args,
                 css::uno::Reference<css::uno::XComponentContext> const & xComponentContext );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XExecutableDialog
    virtual void SAL_CALL setTitle( OUString const & title ) override;
    virtual sal_Int16 SAL_CALL execute() override;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
