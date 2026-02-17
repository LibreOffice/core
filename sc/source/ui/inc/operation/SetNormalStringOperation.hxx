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

namespace sc
{
/** Operation which sets string content to a cell. */
class SetNormalStringOperation : public Operation
{
private:
    ScDocFunc& mrDocFunc;
    ScDocShell& mrDocShell;
    ScAddress const& mrPosition;
    OUString const& mrText;
    bool mbIsNumberFormatSet = false;

    bool runImplementation() override;

public:
    SetNormalStringOperation(ScDocFunc& rDocFunc, ScDocShell& rDocShell, const ScAddress& rPosition,
                             const OUString& rText, bool bApi);
    bool isNumberFormatSet() { return mbIsNumberFormatSet; }
};
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
