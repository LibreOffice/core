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

class ScDocShell;
class ScMarkData;
class ScDocFunc;
enum class InsertDeleteFlags : sal_Int32;

namespace sc
{
/** Operation which deletes content of one or multiple marked cells. */
class DeleteContentOperation : public Operation
{
private:
    ScDocFunc& mrDocFunc;
    ScDocShell& mrDocShell;
    ScMarkData const& mrMark;
    InsertDeleteFlags mnFlags;

    bool runImplementation() override;

public:
    DeleteContentOperation(ScDocFunc& rDocFunc, ScDocShell& rDocShell, const ScMarkData& rMark,
                           InsertDeleteFlags nFlags, bool bRecord, bool bApi);
};
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
