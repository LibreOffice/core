/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "workbookhelper.hxx"
#include "NamedSheetViewFragment.hxx"

#include <vector>

namespace oox::xls::nsv
{
/** Imports named sheet view data into the document model. */
class NamedSheetViewImporter final : public WorkbookHelper
{
    std::vector<NamedSheetViewData> maNamedSheetViews;
    SCTAB mnTab;

public:
    explicit NamedSheetViewImporter(const WorkbookHelper& rHelper, SCTAB nTab);
    void setNamedSheetViews(std::vector<NamedSheetViewData>&& rData);
    void finalizeImport();
};

} // namespace oox::xls::nsv

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
