/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vector>

#include <svx/svdundo.hxx>

#include "SlideLinkManager.hxx"
#include "sddllapi.h"

class SdDrawDocument;

namespace sd
{
/** One operation on the slide links of a document: a link made, taken off or refreshed.

    The action holds the records as they were before the operation. Undo puts them back, and
    redo puts back the records as they stood when the action was first undone. The action does
    not move pages: the pages an operation reads or removes have undo actions of their own.
*/
class SD_DLLPUBLIC UndoSlideLinks final : public SdrUndoAction
{
public:
    UndoSlideLinks(SdDrawDocument& rDoc, const OUString& rComment,
                   std::vector<SlideLinkRecord> aOldLinks);

    virtual void Undo() override;
    virtual void Redo() override;
    virtual OUString GetComment() const override;

private:
    std::vector<SlideLinkRecord> maOldLinks;
    std::vector<SlideLinkRecord> maNewLinks;
    OUString maComment;
    bool mbNewCaptured = false;
};

} // namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
