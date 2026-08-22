/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#ifndef INCLUDED_OOX_PPT_COMMENTS_HXX
#define INCLUDED_OOX_PPT_COMMENTS_HXX

#include <string_view>
#include <vector>

#include <com/sun/star/util/DateTime.hpp>
#include <rtl/ustring.hxx>
#include <sal/types.h>

namespace oox::ppt {

/** One author of a threaded comment, from the modern comment author list.

    The id is a brace-wrapped identifier that the comment entries refer to. It
    is not the small integer the older comment author list uses.
 */
struct ModernCommentAuthor
{
    OUString maId;
    OUString maName;
    OUString maInitials;
};

/** The root of a threaded comment, or one reply to such a root.

    maCreated holds the time the entry was written, as an ISO 8601 date-time in
    UTC written without a marker. mnParentIndex is the position of the thread
    root in the list the entry belongs to, and is -1 for a root.
 */
struct ModernComment
{
    OUString maAuthorId;
    OUString maCreated;
    OUString maText;
    sal_Int32 mnParentIndex = -1;
};

/** The authors of the threaded comments of one presentation. */
class ModernCommentAuthorList
{
public:
    std::vector<ModernCommentAuthor> maAuthors;

    /// The display name the id names, or an empty string when the id is unknown.
    OUString getName(std::u16string_view rAuthorId) const;
    /// The initials the id names, or an empty string when the id is unknown.
    OUString getInitials(std::u16string_view rAuthorId) const;
};

struct CommentAuthor
{
    OUString clrIdx;
    OUString id;
    OUString initials;
    OUString lastIdx;
    OUString name;
};

class CommentAuthorList
{
    private:
        std::vector<CommentAuthor> cmAuthorLst;

    public:
        void setValues(const CommentAuthorList& list);

        void addAuthor(const CommentAuthor& _author)
        {
            cmAuthorLst.push_back(_author);
        }

        friend class Comment;
};

class Comment
{
    private:
        OUString authorId;
        OUString dt;
        OUString idx;
        OUString x;
        OUString y;
        OUString text;
        css::util::DateTime aDateTime;

        void setDateTime (const OUString& datetime);

    public:
        void setAuthorId(const OUString& _aId)
        {
            authorId = _aId;
        }
        void setdt(const OUString& _dt)
        {
            dt=_dt;
            setDateTime(_dt);
        }
        void setidx(const OUString& _idx)
        {
            idx=_idx;
        }
        void setPoint(const OUString& _x, const OUString& _y)
        {
            x=_x;
            y=_y;
        }
        void setText(const OUString& _text)
        {
            text = _text;
        }
        const OUString& get_text() const
        {
            return text;
        }
        const css::util::DateTime& getDateTime() const
        {
            return aDateTime;
        }
        sal_Int32 getIntX() const
        {
            return x.toInt32();
        }
        sal_Int32 getIntY() const
        {
            return y.toInt32();
        }
        OUString getAuthor ( const CommentAuthorList& list );
        OUString getInitials ( const CommentAuthorList& list );
};

class CommentList
{
    public:
        std::vector<Comment> cmLst;
        int getSize () const
        {
            return static_cast<int>(cmLst.size());
        }
        const Comment& getCommentAtIndex (int index);
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
