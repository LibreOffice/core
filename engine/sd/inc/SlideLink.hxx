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

#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <string_view>

namespace tools
{
class JsonWriter;
}
class SdDrawDocument;

namespace sd
{
/** A page inserted as a link to another presentation keeps two names: a reference that identifies
    the source document, and the name of the page of that source it was made from.

    The reference names the source under a scheme of our own, because it identifies a document
    rather than addressing one. Nothing resolves it: a refresh is handed a local file holding the
    pages to read, and the reference is what says which pages that file belongs to.
*/
class SlideLink
{
public:
    /** The reference a page linked to the source document rSourceName records.

        rSourceName is the source document as the user knows it. It is escaped and then decoded back
        to the form a saved reference has when it is read again: the escapes of the characters that
        would otherwise be read as delimiters stay, and a character outside ASCII stays that
        character. A reference built this way is the same string before a save and after the next
        open, which is what matching a page against its source rests on.
    */
    static OUString MakeSourceReference(const OUString& rSourceName);

    /** The source document rReference names, as the user knows it.

        Empty when rReference names no source document, which is the case for a page linked to a
        file by its path.
    */
    static OUString GetSourceName(std::u16string_view rReference);

    /** The file the pages linked to rReference are read from, or empty when nothing reads them.

        A source document that has a file staged for it in rDoc is read from that file. A reference
        that names a source document with no staged file is read from nothing at all, since such a
        reference identifies a document rather than addressing one. Any other reference names the
        file to read itself, and a document served by a kit reads it only when it is a file on this
        machine, so that resolving a link needs no network.
    */
    static OUString GetSourceFile(const SdDrawDocument& rDoc, const OUString& rReference);

    /** Writes the pages of rDoc that are linked to a source document, grouped by source:

        {"links":[{"source":"<source document>",
                   "slides":[{"part":<page id>,"name":"<source page>"},...]},...]}

        The sources come in the order their first page appears and the pages of one source in
        document order. A page id is the unique identifier the linked page holds now, and a source
        page is named as the user sees it in the source document. A page linked to a file by its path
        is left out, since it names no source document.
    */
    static void WriteLinks(const SdDrawDocument& rDoc, tools::JsonWriter& rJsonWriter);

    /** Refreshes the pages of rDoc linked to the source document rSourceName from rFileUrl.

        rFileUrl is a file on this machine, named by a file: URL, holding the pages to read. It is a
        file staged for this one refresh rather than the source document itself, so each page keeps
        the reference to rSourceName it already records and stays linked to it. Every page keeps its
        position and its name as well, and a page whose source page is missing from the file keeps
        the content it holds. Each refreshed page is the page read for it, so it holds a new unique
        identifier afterwards. The whole refresh is one undo step.

        @return the number of pages refreshed, or -1 when no page of rDoc is linked to rSourceName,
                when rFileUrl is not a file on this machine, or when the file could not be read.
    */
    static sal_Int32 Refresh(SdDrawDocument& rDoc, const OUString& rSourceName,
                             const OUString& rFileUrl);

    /** Takes the source document off the page at nIndex in the standard page list of rDoc.

        The page keeps the content it holds and becomes a page of this document alone, so a refresh
        of the source it came from leaves it as it is. Taking the source off is one undo action, and
        undoing it gives the page its source back. Only a page that names a source document, one
        WriteLinks reports, has a source to take off; a page linked to a file by its path keeps what
        it records.

        @return true when the page named a source document and does not any more, false for every
                other page and for an index that names no page.
    */
    static bool Break(SdDrawDocument& rDoc, sal_Int32 nIndex);

    /** Takes the source document off the page at nIndex, which somebody just changed.

        A page holds the content its source gave it until somebody edits it, and an edited page is
        the document's own. The undo action goes with the edit being recorded, so one undo gives the
        page both its old content and its source back. A change with no undo step in hand is the
        engine's own work on the page rather than an edit, and leaves the source alone, as does any
        change while the document is reading pages or moving them about.
    */
    static void BreakOnEdit(SdDrawDocument& rDoc, sal_Int32 nIndex);
};

} // namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
