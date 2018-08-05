/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <oox/ppt/comments.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <rtl/math.hxx>

namespace oox { namespace ppt {

void CommentAuthorList::setValues(const CommentAuthorList& list)
{
    for (auto const& author : list.cmAuthorLst)
    {
        CommentAuthor temp;
        // TODO JNA : why not doing push_back at the end instead of using back()?
        cmAuthorLst.push_back(temp);
        cmAuthorLst.back().clrIdx = author.clrIdx;
        cmAuthorLst.back().id = author.id;
        cmAuthorLst.back().initials = author.initials;
        cmAuthorLst.back().lastIdx = author.lastIdx;
        cmAuthorLst.back().name = author.name;
    }
}

//DateTime is saved as : 2013-01-10T15:53:26.000
void Comment::setDateTime (const OUString& _datetime)
{
    OUString datetime = _datetime;
    aDateTime.Year = datetime.getToken(0,'-').toInt32();
    aDateTime.Month = datetime.getToken(1,'-').toUInt32();
    aDateTime.Day = datetime.getToken(2,'-').toUInt32();
    datetime = datetime.getToken(1,'T');
    aDateTime.Hours = datetime.getToken(0,':').toUInt32();
    aDateTime.Minutes = datetime.getToken(1,':').toUInt32();
    double seconds = datetime.getToken(2,':').toDouble();
    aDateTime.Seconds = floor(seconds);
    seconds -= aDateTime.Seconds;
    aDateTime.NanoSeconds = ::rtl::math::round(seconds * 1000000000);
    const int secondsOverflow = (aDateTime.Seconds == 60) ? 61 : 60;
    // normalise time part of aDateTime
    if (aDateTime.NanoSeconds == 1000000000)
    {
        aDateTime.NanoSeconds = 0;
        ++aDateTime.Seconds;
    }
    if (aDateTime.Seconds == secondsOverflow)
    {
        aDateTime.Seconds = 0;
        ++aDateTime.Minutes;
    }
    if (aDateTime.Minutes == 60)
    {
        aDateTime.Minutes = 0;
        ++aDateTime.Hours;
    }
    // if overflow goes into date, I give up
}

OUString Comment::getAuthor ( const CommentAuthorList& list )
{
    const sal_Int32 nId = authorId.toInt32();
    for (auto const& author : list.cmAuthorLst)
    {
        if(author.id.toInt32() == nId)
            return author.name;
    }
    return OUString("Anonymous");
}

const Comment& CommentList::getCommentAtIndex (int index)
{
    if(index >= static_cast<int>(cmLst.size()) || index < 0)
        throw css::lang::IllegalArgumentException();

    return cmLst.at(index)
;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
