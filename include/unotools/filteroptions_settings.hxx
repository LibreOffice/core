/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <rtl/ustring.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>

#include <officecfg/Office/UI.hxx>

namespace utl
{
// Read-only access for elements under org.openoffice.Office.UI/FilePicker/ShowFilterDialog
inline css::uno::Reference<css::beans::XPropertySet>
getSettingsForFilterOptions(const OUString& filter)
{
    auto xNameAccess = officecfg::Office::UI::FilePicker::ShowFilterDialog::get();
    if (xNameAccess->hasByName(filter))
        return xNameAccess->getByName(filter).query<css::beans::XPropertySet>();
    return {};
}

// Write access for elements under org.openoffice.Office.UI/FilePicker/ShowFilterDialog
inline css::uno::Reference<css::beans::XPropertySet>
getSettingsForFilterOptions(const OUString& filter,
                            const std::shared_ptr<comphelper::ConfigurationChanges>& batch)
{
    auto xNameContainer = officecfg::Office::UI::FilePicker::ShowFilterDialog::get(batch);
    if (!xNameContainer->hasByName(filter))
    {
        auto xFactory = xNameContainer.queryThrow<css::lang::XSingleServiceFactory>();
        xNameContainer->insertByName(filter, css::uno::Any(xFactory->createInstance()));
    }
    return xNameContainer->getByName(filter).query<css::beans::XPropertySet>();
}

inline bool isShowFilterOptionsDialog(const OUString& filter)
{
    if (auto settings = getSettingsForFilterOptions(filter))
        return settings->getPropertyValue(u"show"_ustr).get<bool>();
    return true; // default is show
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
