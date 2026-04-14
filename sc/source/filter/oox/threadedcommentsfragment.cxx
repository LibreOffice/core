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

#include <sal/config.h>

#include <threadedcommentsfragment.hxx>

#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <addressconverter.hxx>
#include <document.hxx>
#include <postit.hxx>

#include <map>

namespace oox::xls
{
ThreadedCommentsFragment::ThreadedCommentsFragment(const WorksheetHelper& rHelper,
                                                   const OUString& rFragmentPath)
    : WorksheetFragmentBase(rHelper, rFragmentPath)
{
}

core::ContextHandlerRef ThreadedCommentsFragment::onCreateContext(sal_Int32 nElement,
                                                                  const AttributeList& rAttribs)
{
    switch (getCurrentElement())
    {
        case core::XML_ROOT_CONTEXT:
            if (nElement == XTHREADED_TOKEN(ThreadedComments))
                return this;
            break;
        case XTHREADED_TOKEN(ThreadedComments):
            if (nElement == XTHREADED_TOKEN(threadedComment))
            {
                mpCurrentEntry = &maEntries.emplace_back();
                mpCurrentEntry->maRef = rAttribs.getXString(XML_ref, {});
                mpCurrentEntry->maId = rAttribs.getXString(XML_id, {});
                mpCurrentEntry->maPersonId = rAttribs.getXString(XML_personId, {});
                mpCurrentEntry->maDateTime = rAttribs.getXString(XML_dT, {});
                mpCurrentEntry->maParentId = rAttribs.getXString(XML_parentId, {});
                mpCurrentEntry->mbDone = rAttribs.getBool(XML_done, false);
                return this;
            }
            break;
        case XTHREADED_TOKEN(threadedComment):
            if (nElement == XTHREADED_TOKEN(text))
                return this; // collect text in onCharacters()
            break;
    }
    return {};
}

void ThreadedCommentsFragment::onCharacters(const OUString& rChars)
{
    if (isCurrentElement(XTHREADED_TOKEN(text)) && mpCurrentEntry)
        mpCurrentEntry->maText += rChars;
}

void ThreadedCommentsFragment::onEndElement()
{
    if (isCurrentElement(XTHREADED_TOKEN(threadedComment)))
        mpCurrentEntry = nullptr;
    else if (isCurrentElement(XTHREADED_TOKEN(ThreadedComments)))
        attachToNotes();
}

void ThreadedCommentsFragment::attachToNotes()
{
    ScDocument& rDoc = getScDocument();

    // Separate root comments from replies. Excel uses flat threads: all replies
    // reference the root comment's id in their parentId. Nested replies (where a
    // reply references another reply) are not supported by Excel and are ignored here.
    std::map<OUString, ThreadedCommentImportEntry*> aRootById;
    std::map<OUString, std::vector<ThreadedCommentImportEntry*>> aRepliesByParentId;

    for (auto& rEntry : maEntries)
    {
        if (rEntry.maParentId.isEmpty())
            aRootById[rEntry.maId] = &rEntry;
        else
            aRepliesByParentId[rEntry.maParentId].push_back(&rEntry);
    }

    // For each root entry, find the matching ScPostIt and attach threaded data.
    for (auto & [ rId, pRoot ] : aRootById)
    {
        // Convert cell reference to address.
        ScRange aRange;
        AddressConverter::convertToCellRangeUnchecked(aRange, pRoot->maRef, getSheetIndex(), rDoc);
        ScPostIt* pNote = rDoc.GetNote(aRange.aStart);
        if (!pNote)
            continue; // Match Excel: threaded comment without a legacy comment is ignored

        auto pData = std::make_unique<ScThreadedCommentData>();
        pData->maRoot.maGuid = pRoot->maId;
        pData->maRoot.maPersonId = pRoot->maPersonId;
        pData->maRoot.maDateTime = pRoot->maDateTime;
        pData->maRoot.maText = pRoot->maText;
        pData->mbDone = pRoot->mbDone;

        // Attach replies.
        auto itReplies = aRepliesByParentId.find(rId);
        if (itReplies != aRepliesByParentId.end())
        {
            for (const auto* pReply : itReplies->second)
            {
                ScThreadedCommentEntry aReplyEntry;
                aReplyEntry.maGuid = pReply->maId;
                aReplyEntry.maPersonId = pReply->maPersonId;
                aReplyEntry.maDateTime = pReply->maDateTime;
                aReplyEntry.maText = pReply->maText;
                pData->maReplies.push_back(std::move(aReplyEntry));
            }
        }

        // Replace the legacy placeholder author (tc={guid}) with the actual person name.
        const ScPersonData* pPerson = rDoc.GetPersonById(pRoot->maPersonId);
        if (pPerson)
            pNote->SetAuthor(pPerson->maDisplayName);

        pNote->SetThreadedCommentData(std::move(pData));
    }
}

} // namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
