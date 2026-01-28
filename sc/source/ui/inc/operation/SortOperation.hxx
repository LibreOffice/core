/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "Operation.hxx"
#include <sal/types.h>
#include <address.hxx>

class ScDocShell;
class ScMarkData;
class ScDocFunc;
struct ScSortParam;
enum class InsertDeleteFlags : sal_Int32;

namespace sc
{
/** Operation which sorts cells in a sheet. */
class SortOperation : public Operation
{
private:
    ScDocShell& mrDocShell;
    SCTAB mnTab;
    ScSortParam const& mrSortParam;
    bool mbPaint;

    bool runImplementation() override;

public:
    SortOperation(ScDocShell& rDocShell, SCTAB nTab, ScSortParam const& rSortParam, bool bRecord,
                  bool bPaint, bool bApi);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
