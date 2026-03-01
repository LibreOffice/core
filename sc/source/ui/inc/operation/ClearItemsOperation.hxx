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

namespace sc
{
/** Operation which clears specific items from one or multiple marked cells. */
class ClearItemsOperation : public Operation
{
private:
    ScDocShell& mrDocShell;
    ScMarkData const& mrMark;
    const sal_uInt16* mpWhich;

    bool runImplementation() override;

public:
    ClearItemsOperation(ScDocShell& rDocShell, const ScMarkData& rMark, const sal_uInt16* pWhich,
                        bool bApi);
};
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
