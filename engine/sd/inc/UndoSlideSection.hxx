/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "SlideSectionManager.hxx"
#include "sddllapi.h"
#include <svx/svdundo.hxx>
#include <vector>

class SdDrawDocument;

namespace sd
{
/** Undo action for slide section metadata changes (add/remove/rename/move).
 *
 *  Captures the full sections vector before the change; Undo restores it,
 *  and Redo re-applies the new state captured on first invocation.
 *
 *  Note: this action does not move slides. For section moves that also
 *  reorder pages, the page-reorder undo is recorded separately by the
 *  underlying MovePages() call.
 */
class SD_DLLPUBLIC UndoSlideSection final : public SdrUndoAction
{
public:
    UndoSlideSection(SdDrawDocument& rDoc, const OUString& rComment);

    virtual void Undo() override;
    virtual void Redo() override;
    virtual OUString GetComment() const override;

private:
    std::vector<SlideSection> maOldSections;
    std::vector<SlideSection> maNewSections;
    OUString maComment;
    bool mbNewCaptured = false;
};

} // namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
