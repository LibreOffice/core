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
#include <boost/algorithm/string.hpp> //split function to tokenize for date time

#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

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
        void setValues(const CommentAuthorList& list)
        {
            std::vector<CommentAuthor>::const_iterator it;
            for(it = list.cmAuthorLst.begin(); it != list.cmAuthorLst.end(); ++it)
            {
                CommentAuthor temp;
                cmAuthorLst.push_back(temp);
                cmAuthorLst.back().clrIdx = it->clrIdx;
                cmAuthorLst.back().id = it->id;
                cmAuthorLst.back().initials = it->initials;
                cmAuthorLst.back().lastIdx = it->lastIdx;
                cmAuthorLst.back().name = it->name;
            }
        }

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

        //DateTime is saved as : 2013-01-10T15:53:26.000
        void setDateTime (::rtl::OUString datetime)
        {
            std::string _datetime = rtl::OUStringToOString(datetime, RTL_TEXTENCODING_UTF8).getStr();
            std::vector<std::string> _dt;
            boost::split( _dt, _datetime, boost::is_any_of( "-:T" ) );
            aDateTime.Year = atoi(_dt.at(0).c_str());
            aDateTime.Month = atoi(_dt.at(1).c_str());
            aDateTime.Day = atoi(_dt.at(2).c_str());
            aDateTime.Hours = atoi(_dt.at(3).c_str());
            aDateTime.Minutes = atoi(_dt.at(4).c_str());
            aDateTime.HundredthSeconds = atoi(_dt.at(5).c_str());
            std::vector<std::string>::iterator i;
        }

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
        void setText(std::string _text)
        {
            text = rtl::OUString::createFromAscii (  _text.c_str() );
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
        int getIntX()
        {
            std::string temp = rtl::OUStringToOString(get_X(), RTL_TEXTENCODING_UTF8).getStr();
            return atoi(temp.c_str());
        }
        int getIntY()
        {
            std::string temp = rtl::OUStringToOString(get_Y(), RTL_TEXTENCODING_UTF8).getStr();
            return atoi(temp.c_str());
        }
        OUString getAuthor ( const CommentAuthorList& list )
        {
            std::string temp = rtl::OUStringToOString(authorId, RTL_TEXTENCODING_UTF8).getStr();
            int aId = atoi(temp.c_str());
            std::vector<CommentAuthor>::const_iterator it;
            for(it = list.cmAuthorLst.begin(); it != list.cmAuthorLst.end(); ++it)
            {
                temp = rtl::OUStringToOString(it->id, RTL_TEXTENCODING_UTF8).getStr();

                int list_aId = atoi(temp.c_str());
                std::string temp_a =rtl::OUStringToOString(it->name, RTL_TEXTENCODING_UTF8).getStr();
                if(list_aId == aId)
                    return it->name;
            }
            return OUString("Anonymous");
        }
};

class CommentList
{
    public:
        std::vector<Comment> cmLst;
        int getSize ()
        {
            return (int)cmLst.size();
        }
        const Comment& getCommentAtIndex (int index)
        {
            if(index < (int)cmLst.size() && index >= 0)
                return cmLst.at(index);
            else
                throw css::lang::IllegalArgumentException();
        }
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
