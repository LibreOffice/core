/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#ifndef OOX_PPT_COMMENTS_HXX
#define OOX_PPT_COMMENTS_HXX

#include <vector>
#include <com/sun/star/util/DateTime.hpp>

namespace oox { namespace ppt {

struct CommentAuthor
{
    ::rtl::OUString clrIdx;
    ::rtl::OUString id;
    ::rtl::OUString initials;
    ::rtl::OUString lastIdx;
    ::rtl::OUString name;
};

class CommentAuthorList
{
    private:
        std::vector<CommentAuthor> cmAuthorLst;

    public:
        void setValues(const CommentAuthorList& list);

        const std::vector<CommentAuthor>& getCmAuthorLst() const
        {
            return cmAuthorLst;
        }

        void addAuthor(const CommentAuthor& _author)
        {
            cmAuthorLst.push_back(_author);
        }

        friend class Comment;
};

class Comment
{
    private:
        ::rtl::OUString authorId;
        ::rtl::OUString dt;
        ::rtl::OUString idx;
        ::rtl::OUString x;
        ::rtl::OUString y;
        ::rtl::OUString text;
        ::com::sun::star::util::DateTime aDateTime;

        void setDateTime (::rtl::OUString datetime);

    public:
        void setAuthorId(const ::rtl::OUString& _aId)
        {
            authorId = _aId;
        }
        void setdt(const ::rtl::OUString& _dt)
        {
            dt=_dt;
            setDateTime(_dt);
        }
        void setidx(const ::rtl::OUString& _idx)
        {
            idx=_idx;
        }
        void setPoint(const ::rtl::OUString& _x, const ::rtl::OUString& _y)
        {
            x=_x;
            y=_y;
        }
        void setText(const rtl::OUString& _text)
        {
            text = _text;
        }
        ::rtl::OUString getAuthorId()
        {
            return authorId;
        }
        ::rtl::OUString getdt()
        {
            return dt;
        }
        ::rtl::OUString getidx()
        {
            return idx;
        }
        ::rtl::OUString get_X()
        {
            return x;
        }
        ::rtl::OUString get_Y()
        {
            return y;
        }
        ::rtl::OUString get_text()
        {
            return text;
        }
        ::com::sun::star::util::DateTime getDateTime()
        {
            return aDateTime;
        }
        sal_Int32 getIntX()
        {
            return x.toInt32();
        }
        sal_Int32 getIntY()
        {
            return y.toInt32();
        }
        ::rtl::OUString getAuthor ( const CommentAuthorList& list );
};

class CommentList
{
    public:
        std::vector<Comment> cmLst;
        int getSize ()
        {
            return (int)cmLst.size();
        }
        const Comment& getCommentAtIndex (int index);
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
