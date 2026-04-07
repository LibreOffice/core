/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the LibreOffice contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include "excelhandlers.hxx"

#include <vector>

namespace oox::xls
{
/** Intermediate model for a single threadedComment element during import. */
struct ThreadedCommentImportEntry
{
    OUString maRef; /// Cell reference (e.g. "A2").
    OUString maId; /// Unique GUID.
    OUString maPersonId; /// Person GUID.
    OUString maDateTime; /// xsd:dateTime.
    OUString maParentId; /// Parent GUID (empty = root).
    bool mbDone = false; /// Resolved flag.
    OUString maText; /// Plain text content.
};

/** Fragment handler for the threaded-comments part
    (xl/threadedComments/threadedComment*.xml).
    Imports threaded comment data and attaches it to existing ScPostIt notes
    ([MS-XLSX] section 2.1.20). */
class ThreadedCommentsFragment final : public WorksheetFragmentBase
{
public:
    explicit ThreadedCommentsFragment(const WorksheetHelper& rHelper,
                                      const OUString& rFragmentPath);

private:
    virtual oox::core::ContextHandlerRef onCreateContext(sal_Int32 nElement,
                                                         const AttributeList& rAttribs) override;
    virtual void onCharacters(const OUString& rChars) override;
    virtual void onEndElement() override;

    /** Attaches collected threaded comment data to existing ScPostIt notes. */
    void attachToNotes();

    std::vector<ThreadedCommentImportEntry> maEntries;
    ThreadedCommentImportEntry* mpCurrentEntry = nullptr;
};

} // namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
