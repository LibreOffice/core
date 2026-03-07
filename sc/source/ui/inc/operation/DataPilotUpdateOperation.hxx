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

class ScDocShell;
class ScDPObject;

namespace sc
{
/** Data pilot update is used for various pivot table manipulation. */
class DataPilotUpdateOperation : public Operation
{
private:
    ScDocShell& mrDocShell;
    ScDPObject* mpOldObj;
    ScDPObject const* mpNewObj;
    bool mbAllowMove;

    bool runImplementation() override;

public:
    DataPilotUpdateOperation(ScDocShell& rDocShell, ScDPObject* pOldObj, ScDPObject const* pNewObj,
                             bool bRecord, bool bApi, bool bAllowMove);
};
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
