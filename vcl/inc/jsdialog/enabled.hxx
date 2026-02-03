/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <string_view>
#include <o3tl/sorted_vector.hxx>

namespace jsdialog
{
/// used to not warn about it
bool isIgnored(std::u16string_view rUIFile);
bool isBuilderEnabled(std::u16string_view rUIFile, bool bMobile);
bool isBuilderEnabledForPopup(std::u16string_view rUIFile);
bool isBuilderEnabledForMenu(std::u16string_view rUIFile);
bool isBuilderEnabledForSidebar(std::u16string_view rUIFile);
bool isBuilderEnabledForAddressInput(std::u16string_view rUIFile);
bool isBuilderEnabledForFormulabar(std::u16string_view rUIFile);
bool isBuilderEnabledForNavigator(std::u16string_view rUIFile);
bool isBuilderEnabledForQuickFind(std::u16string_view rUIFile);
bool isInterimBuilderEnabledForNotebookbar(std::u16string_view rUIFile);

// returns vector of writer dialog .ui files not seen that should be seen to have complete coverage
std::vector<OUString> completeWriterDialogList(const o3tl::sorted_vector<OUString>& entries);
// returns vector of writer sidebar .ui files not seen that should be seen to have complete coverage
std::vector<OUString> completeWriterSidebarList(const o3tl::sorted_vector<OUString>& entries);
// returns vector of calc dialog .ui files not seen that should be seen to have complete coverage
std::vector<OUString> completeCalcDialogList(const o3tl::sorted_vector<OUString>& entries);
// returns vector of calc sidebar .ui files not seen that should be seen to have complete coverage
std::vector<OUString> completeCalcSidebarList(const o3tl::sorted_vector<OUString>& entries);
// returns vector of common/shared dialog .ui files not seen that should be seen to have complete coverage
std::vector<OUString> completeCommonDialogList(const o3tl::sorted_vector<OUString>& entries,
                                               bool linguisticDataAvailable);
// returns vector of common/shared sidebar .ui files not seen that should be seen to have complete coverage
std::vector<OUString> completeCommonSidebarList(const o3tl::sorted_vector<OUString>& entries);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
