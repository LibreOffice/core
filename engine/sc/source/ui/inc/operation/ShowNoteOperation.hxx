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
#include <address.hxx>

class ScDocShell;

namespace sc
{
/** Operation which shows or hides the caption of a cell note. */
class ShowNoteOperation : public Operation
{
private:
    ScDocShell& mrDocShell;
    ScAddress maPos;
    bool mbShow;

    bool runImplementation() override;

public:
    ShowNoteOperation(ScDocShell& rDocShell, const ScAddress& rPos, bool bShow);
};
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
