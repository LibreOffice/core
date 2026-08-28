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
#include <string_view>

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
};

} // namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
