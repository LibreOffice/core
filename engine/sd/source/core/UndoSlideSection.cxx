/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <UndoSlideSection.hxx>
#include <drawdoc.hxx>

namespace sd
{
UndoSlideSection::UndoSlideSection(SdDrawDocument& rDoc, const OUString& rComment)
    : SdrUndoAction(rDoc)
    , maOldSections(rDoc.GetSectionManager().GetSectionsSnapshot())
    , maComment(rComment)
{
}

void UndoSlideSection::Undo()
{
    auto& rDoc = static_cast<SdDrawDocument&>(m_rMod);
    if (!mbNewCaptured)
    {
        maNewSections = rDoc.GetSectionManager().GetSectionsSnapshot();
        mbNewCaptured = true;
    }
    rDoc.GetSectionManager().RestoreSectionsSnapshot(maOldSections);
}

void UndoSlideSection::Redo()
{
    auto& rDoc = static_cast<SdDrawDocument&>(m_rMod);
    rDoc.GetSectionManager().RestoreSectionsSnapshot(maNewSections);
}

OUString UndoSlideSection::GetComment() const { return maComment; }

} // namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
