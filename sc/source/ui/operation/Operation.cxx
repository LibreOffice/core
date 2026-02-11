/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/Operation.hxx>
#include <SheetViewOperationsTester.hxx>
#include <docsh.hxx>
#include <sal/log.hxx>

namespace sc
{
bool Operation::checkSheetViewProtection()
{
    sc::SheetViewOperationsTester aSheetViewTester(ScDocShell::GetViewData());
    return aSheetViewTester.check(meType);
}

bool Operation::run()
{
    SAL_INFO("sc",
             "Running operation '" << SheetViewOperationsTester::operationName(meType) << "'.");
    bool bResult = runImplementation();
    SAL_INFO("sc", "Operation '" << SheetViewOperationsTester::operationName(meType)
                                 << (bResult ? "' succeded." : "' failed."));
    return bResult;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
