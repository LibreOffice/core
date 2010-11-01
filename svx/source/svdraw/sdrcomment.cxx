/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <svx/sdrcomment.hxx>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
