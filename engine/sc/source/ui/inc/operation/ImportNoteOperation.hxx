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

#include <tools/gen.hxx>

#include <memory>

class ScDocShell;
class GenerateNoteCaption;

namespace sc
{
/** Operation which installs a note caption at a cell from a deferred
 *  generator, used by the load path.
 */
class ImportNoteOperation : public Operation
{
private:
    ScDocShell& mrDocShell;
    ScAddress maPos;
    std::unique_ptr<GenerateNoteCaption> mxGenerator;
    tools::Rectangle maCaptionRect;
    bool mbShown;

    bool runImplementation() override;

public:
    ImportNoteOperation(ScDocShell& rDocShell, const ScAddress& rPos,
                        std::unique_ptr<GenerateNoteCaption> xGenerator,
                        const tools::Rectangle& rCaptionRect, bool bShown);
};
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
