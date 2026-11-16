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

#include <optional>
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

/** One change to one source document a document keeps: the source kept, renamed or kept no more.

    The action holds the record of that source as it was before the change and as it is after it,
    each empty when the source is not kept, and the place the source has in the list. Undo puts
    the first back and redo the second. The other kept sources stay as they are.
*/
class SD_DLLPUBLIC UndoSlideLinkSources final : public SdrUndoAction
{
public:
    UndoSlideLinkSources(SdDrawDocument& rDoc, const OUString& rComment, const OUString& rSource,
                         size_t nPosition, std::optional<SlideLinkSource> oOldSource,
                         std::optional<SlideLinkSource> oNewSource);

    virtual void Undo() override;
    virtual void Redo() override;
    virtual OUString GetComment() const override;

private:
    OUString maSource;
    size_t mnPosition;
    std::optional<SlideLinkSource> moOldSource;
    std::optional<SlideLinkSource> moNewSource;
    OUString maComment;
};

} // namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
