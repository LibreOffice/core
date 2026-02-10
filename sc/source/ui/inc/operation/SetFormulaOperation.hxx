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
#include <rtl/ustring.hxx>

class ScDocShell;
class ScAddress;
class ScDocFunc;
class ScFormulaCell;

namespace sc
{
/** Operation which sets a formula to a cell. */
class SetFormulaOperation : public Operation
{
private:
    ScDocFunc& mrDocFunc;
    ScDocShell& mrDocShell;
    ScAddress const& mrPosition;
    ScFormulaCell* mpFormulaCell;

    bool runImplementation() override;

public:
    SetFormulaOperation(ScDocFunc& rDocFunc, ScDocShell& rDocShell, const ScAddress& rPosition,
                        ScFormulaCell* pFormulaCell, bool bApi);
};

} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
