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

#ifndef INCLUDED_OOX_PPT_MODERNCOMMENTSFRAGMENTHANDLER_HXX
#define INCLUDED_OOX_PPT_MODERNCOMMENTSFRAGMENTHANDLER_HXX

#include <oox/core/fragmenthandler2.hxx>
#include <oox/dllapi.h>
#include <oox/ppt/comments.hxx>

namespace oox
{
namespace core
{
class XmlFilterBase;
}

namespace ppt
{
/** Reads the author list the threaded comments of a presentation refer to. */
class OOX_DLLPUBLIC ModernCommentAuthorsFragmentHandler final : public core::FragmentHandler2
{
public:
    ModernCommentAuthorsFragmentHandler(core::XmlFilterBase& rFilter, const OUString& rFragmentPath,
                                        ModernCommentAuthorList& rAuthorList);
    virtual ~ModernCommentAuthorsFragmentHandler() override;
    virtual core::ContextHandlerRef onCreateContext(sal_Int32 nElement,
                                                    const AttributeList& rAttribs) override;

private:
    ModernCommentAuthorList& mrAuthorList;
};

/** Reads the threaded comments of one slide.

    The entries come out in the order the file lists them, each thread root
    followed by its replies.
 */
class OOX_DLLPUBLIC ModernCommentsFragmentHandler final : public core::FragmentHandler2
{
public:
    ModernCommentsFragmentHandler(core::XmlFilterBase& rFilter, const OUString& rFragmentPath);
    virtual ~ModernCommentsFragmentHandler() override;
    virtual core::ContextHandlerRef onCreateContext(sal_Int32 nElement,
                                                    const AttributeList& rAttribs) override;
    virtual void onCharacters(const OUString& rChars) override;
    virtual void onEndElement() override;

    const std::vector<ModernComment>& getComments() const { return maComments; }

private:
    std::vector<ModernComment> maComments;
    /// Position in maComments of the entry the text being read belongs to, or -1.
    sal_Int32 mnCurrentIndex = -1;
    /// Position in maComments of the root of the thread being read, or -1.
    sal_Int32 mnRootIndex = -1;
    /// True while inside the text body of an entry.
    bool mbInTextBody = false;
};
} // namespace ppt
} // namespace oox

#endif // INCLUDED_OOX_PPT_MODERNCOMMENTSFRAGMENTHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
