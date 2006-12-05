/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdrcomment.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-05 12:13:46 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef _SDR_COMMENT_HXX
#include "sdrcomment.hxx"
#endif

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    Comment::Comment(
        sal_uInt32 nID,
        Date aCreationDate,
        const ::rtl::OUString& rUserName,
        const ::rtl::OUString& rText,
        const basegfx::B2DPoint& rPosition)
    :   mnID(nID),
        maCreationDate(aCreationDate),
        maUserName(rUserName),
        maText(rText),
        maPosition(rPosition)
    {
    }

    Comment::~Comment()
    {
    }

    sal_Bool Comment::operator==(const Comment& rCandidate) const
    {
        return (
            mnID == rCandidate.mnID
            && maCreationDate == rCandidate.maCreationDate
            && maUserName == rCandidate.maUserName
            && maText == rCandidate.maText
            && maPosition == rCandidate.maPosition);
    }

    void Comment::SetCreationDate(Date aNewDate)
    {
        if(aNewDate != maCreationDate)
        {
            maCreationDate = aNewDate;
        }
    }

    void Comment::SetUserName(const ::rtl::OUString& rNewName)
    {
        if(rNewName != maUserName)
        {
            maUserName = rNewName;
        }
    }

    void Comment::SetText(const ::rtl::OUString& rNewText)
    {
        if(rNewText != maText)
        {
            maText = rNewText;
        }
    }

    void Comment::SetPosition(const basegfx::B2DPoint& rNewPos)
    {
        if(rNewPos != maPosition)
        {
            maPosition = rNewPos;
        }
    }
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
