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

#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <string_view>

#include "sddllapi.h"

class SdDrawDocument;
class SdPage;
struct InsertBookmarkOptions;

namespace sd
{
/** The helpers a link to a slide of another presentation rests on: how a written page names the
    slide it came from, how a slide is named by its position, and what a link may record.
*/
class SlideLink
{
public:
    /** The reference the pages of a written presentation record, which names no document.

        A presentation written out of a document holds pages of its own, named by the position each
        one takes in it, so the name a page carries there says nothing about the slide it was
        written from. Every written page records that slide as a link under this reference, so that
        whoever reads the pages back learns which slide of the writing document each one came from.
        The reference names no document and nothing resolves it.
    */
    static OUString OriginReference();

    /** Whether rReference is a reference under our own scheme, which names no file: the record a
        written page keeps of the slide it came from, or the source an earlier build kept on a
        linked page. There is nothing to resolve behind it.
    */
    static bool IsOwnReference(std::u16string_view rReference);

    /** The slide of the writing document rPage was written from, as that document names it.

        Empty for a page that records none: only a page a document wrote out of itself records the
        slide it came from.
    */
    static OUString GetOriginPage(const SdPage& rPage);

    /** The name a slide holds by the position nPosition in its document, counted from one.

        A slide with no name of its own is known by its position, so a page linked to such a name
        is linked to the position rather than to one slide: an update reads whichever slide stands
        there. It is the name ODF keeps for a slide of no name, so the link is one any reader of
        the format resolves.
    */
    static OUString MakePositionName(sal_Int32 nPosition);

    /** The position rName names, counted from one, or 0 when rName is a name of a slide's own.
    */
    static sal_Int32 GetNamedPosition(const OUString& rName);

    /** rGuid in the braced form a slide keeps its own identifier in, or empty when rGuid holds no
        identifier at all. A value of another shape names no slide or section of a source.
    */
    static OUString NormaliseGuid(std::u16string_view rGuid);

    /** Whether rValue can stand as a value a link records about its source, such as the time the
        source document was last modified or the name of a source section: at most 128 characters,
        none of them a control character. The value is the integrator's or the source's own
        string, so nothing else is asked of its form.
    */
    static bool IsReadableSourceString(std::u16string_view rValue);

    /** Whether rLink can stand as the persistent link of a source document, the value a link
        records as its source: not empty, at most 1024 characters, none of them a control
        character.
    */
    static bool IsReadablePersistentLink(std::u16string_view rLink);

    /// The index in the standard page list of rDoc of the page holding rGuid, or -1 for none.
    static sal_Int32 GetPageIndex(const SdDrawDocument& rDoc, std::u16string_view rGuid);

    /// Whether rUrl names a file on this machine: a file URL with no host.
    static bool IsLocalFile(std::u16string_view rUrl);

    /** The file the pages linked to the file rFileName are read from, or empty when nothing reads
        them.

        A reference under our own scheme names no file. A document served by a kit reads a file on
        this machine and nothing else, so that resolving a link needs no network.
    */
    static OUString GetSourceFile(std::u16string_view rFileName);

    /** Takes the file and the slide rPage records off it, link and all, so that it is a page of
        its document alone.
    */
    static void ClearPageReference(SdPage& rPage);

    /** Inserts the pages rNames of rSource at the page position nInsertPosition of rDoc, as pages
        of rDoc alone. 0xFFFF appends them. An empty rNames inserts every page.

        A page written out of a presentation records the slide it was written from, and that record
        stays with the file: the inserted page carries no link of its own. What links the page to
        its source, when anything does, is a record of the slide link manager.

        @return the number of slides inserted, or -1 when the pages could not be inserted.
    */
    static sal_Int32 InsertPages(SdDrawDocument& rDoc, SdDrawDocument& rSource,
                                 const std::vector<OUString>& rNames,
                                 const InsertBookmarkOptions& rOptions,
                                 sal_uInt16 nInsertPosition);

    /** Takes the link off the page at nIndex in the standard page list of rDoc, which somebody
        just changed.

        A page holds the content its source gave it until somebody edits it, and an edited page is
        the document's own. The undo action goes with the edit being recorded, so one undo gives the
        page both its old content and its link back. A change with no undo step in hand is the
        engine's own work on the page rather than an edit, and leaves the link alone, as does any
        change while the document is reading pages or moving them about.
    */
    static void BreakOnEdit(SdDrawDocument& rDoc, sal_Int32 nIndex);
};

} // namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
