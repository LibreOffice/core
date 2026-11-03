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

#include <UndoSlideLink.hxx>

#include <utility>

#include <drawdoc.hxx>

namespace sd
{
UndoSlideLinks::UndoSlideLinks(SdDrawDocument& rDoc, const OUString& rComment,
                               std::vector<SlideLinkRecord> aOldLinks)
    : SdrUndoAction(rDoc)
    , maOldLinks(std::move(aOldLinks))
    , maComment(rComment)
{
}

void UndoSlideLinks::Undo()
{
    auto& rDoc = static_cast<SdDrawDocument&>(m_rMod);
    if (!mbNewCaptured)
    {
        maNewLinks = rDoc.GetSlideLinkManager().GetSnapshot();
        mbNewCaptured = true;
    }
    rDoc.GetSlideLinkManager().RestoreSnapshot(maOldLinks);
}

void UndoSlideLinks::Redo()
{
    auto& rDoc = static_cast<SdDrawDocument&>(m_rMod);
    rDoc.GetSlideLinkManager().RestoreSnapshot(maNewLinks);
}

OUString UndoSlideLinks::GetComment() const { return maComment; }

} // namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
