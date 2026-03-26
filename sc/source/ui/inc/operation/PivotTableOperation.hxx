/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
/** Base class for pivot table operations, providing common members and utilities. */
class PivotTableOperation : public Operation
{
protected:
    ScDocShell& mrDocShell;

    /** Convert the output and source ranges of a DPObject from sheet view to default view.
     *
     * If the input DP object is not on sheet view, do nothing.
     */
    void convertDPObjectRanges(ScDPObject& rDPObject);

    /** Find the DPObject on the default view tab that corresponds to the given
     *  DPObject, which is on a sheet view tab.
     */
    ScDPObject* findDefaultViewDPObject(ScDPObject const& rDPObject);

    PivotTableOperation(OperationType eType, ScDocShell& rDocShell, bool bRecord, bool bApi);
};
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
