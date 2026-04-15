/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "PivotTableOperation.hxx"

class ScDPObject;

namespace sc
{
/** Replaces a pivot table's layout with settings from a different DPObject. */
class SC_DLLPUBLIC ReplacePivotTableOperation : public PivotTableOperation
{
private:
    ScDPObject* mpDPObject;
    ScDPObject const* mpNewDPObject;
    bool mbAllowMove;

    bool runImplementation() override;

public:
    ReplacePivotTableOperation(ScDocShell& rDocShell, ScDPObject* pDPObject,
                               ScDPObject const* pNewDPObject, bool bRecord, bool bApi,
                               bool bAllowMove);
};
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
