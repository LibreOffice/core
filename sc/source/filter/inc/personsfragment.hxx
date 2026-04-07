/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the LibreOffice contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include "excelhandlers.hxx"

namespace oox::xls
{
/** Fragment handler for the persons part (xl/persons/person.xml).
    Imports person metadata for threaded comments ([MS-XLSX] section 2.1.19). */
class PersonsFragment final : public WorkbookFragmentBase
{
public:
    explicit PersonsFragment(const WorkbookHelper& rHelper, const OUString& rFragmentPath);

private:
    virtual oox::core::ContextHandlerRef onCreateContext(sal_Int32 nElement,
                                                         const AttributeList& rAttribs) override;
};

} // namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
