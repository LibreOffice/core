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

#include <sal/config.h>

#include "unoautopilot.hxx"
#include "groupboxwiz.hxx"
#include "listcombowizard.hxx"
#include "gridwizard.hxx"

// the registration methods

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
extensions_dbp_OGroupBoxWizard_get_implementation(
    css::uno::XComponentContext* context , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(
        new ::dbp::OUnoAutoPilot< ::dbp::OGroupBoxWizard>(
            context,
            u"org.openoffice.comp.dbp.OGroupBoxWizard"_ustr,
            { u"com.sun.star.sdb.GroupBoxAutoPilot"_ustr }
            ));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
extensions_dbp_OListComboWizard_get_implementation(
    css::uno::XComponentContext* context , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(
        new ::dbp::OUnoAutoPilot< ::dbp::OListComboWizard>(
            context,
            u"org.openoffice.comp.dbp.OListComboWizard"_ustr,
            { u"com.sun.star.sdb.ListComboBoxAutoPilot"_ustr }
            ));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
extensions_dbp_OGridWizard_get_implementation(
    css::uno::XComponentContext* context , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(
        new ::dbp::OUnoAutoPilot< ::dbp::OGridWizard>(
            context,
            u"org.openoffice.comp.dbp.OGridWizard"_ustr,
            { u"com.sun.star.sdb.GridControlAutoPilot"_ustr }
            ));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
