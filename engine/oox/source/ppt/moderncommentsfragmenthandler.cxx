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

#include <oox/ppt/moderncommentsfragmenthandler.hxx>

#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>

namespace oox::ppt
{
ModernCommentAuthorsFragmentHandler::ModernCommentAuthorsFragmentHandler(
    core::XmlFilterBase& rFilter, const OUString& rFragmentPath,
    ModernCommentAuthorList& rAuthorList)
    : FragmentHandler2(rFilter, rFragmentPath)
    , mrAuthorList(rAuthorList)
{
}

ModernCommentAuthorsFragmentHandler::~ModernCommentAuthorsFragmentHandler() = default;

core::ContextHandlerRef
ModernCommentAuthorsFragmentHandler::onCreateContext(sal_Int32 nElement,
                                                     const AttributeList& rAttribs)
{
    if (nElement == P188_TOKEN(author))
    {
        ModernCommentAuthor aAuthor;
        aAuthor.maId = rAttribs.getStringDefaulted(XML_id);
        aAuthor.maName = rAttribs.getStringDefaulted(XML_name);
        aAuthor.maInitials = rAttribs.getStringDefaulted(XML_initials);
        mrAuthorList.maAuthors.push_back(aAuthor);
    }
    return this;
}

ModernCommentsFragmentHandler::ModernCommentsFragmentHandler(core::XmlFilterBase& rFilter,
                                                             const OUString& rFragmentPath)
    : FragmentHandler2(rFilter, rFragmentPath)
{
}

ModernCommentsFragmentHandler::~ModernCommentsFragmentHandler() = default;

core::ContextHandlerRef
ModernCommentsFragmentHandler::onCreateContext(sal_Int32 nElement, const AttributeList& rAttribs)
{
    switch (nElement)
    {
        case P188_TOKEN(cm):
        case P188_TOKEN(reply):
        {
            ModernComment aComment;
            aComment.maAuthorId = rAttribs.getStringDefaulted(XML_authorId);
            aComment.maCreated = rAttribs.getStringDefaulted(XML_created);
            if (nElement == P188_TOKEN(reply))
                aComment.mnParentIndex = mnRootIndex;
            maComments.push_back(aComment);
            mnCurrentIndex = static_cast<sal_Int32>(maComments.size()) - 1;
            if (nElement == P188_TOKEN(cm))
                mnRootIndex = mnCurrentIndex;
            break;
        }
        case P188_TOKEN(txBody):
            mbInTextBody = true;
            break;
        case A_TOKEN(p):
            // A paragraph after the first one starts on a new line.
            if (mbInTextBody && mnCurrentIndex >= 0 && !maComments[mnCurrentIndex].maText.isEmpty())
                maComments[mnCurrentIndex].maText += "\n";
            break;
    }
    return this;
}

void ModernCommentsFragmentHandler::onCharacters(const OUString& rChars)
{
    if (mbInTextBody && isCurrentElement(A_TOKEN(t)) && mnCurrentIndex >= 0)
        maComments[mnCurrentIndex].maText += rChars;
}

void ModernCommentsFragmentHandler::onEndElement()
{
    switch (getCurrentElement())
    {
        case P188_TOKEN(txBody):
            mbInTextBody = false;
            break;
        case P188_TOKEN(reply):
            // The text body of the thread root follows the replies, so reading a
            // reply hands the position back to the root it belongs to.
            mnCurrentIndex = mnRootIndex;
            break;
    }
}
} // namespace oox::ppt

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
