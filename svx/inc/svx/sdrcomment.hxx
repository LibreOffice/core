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

#ifndef _SDR_COMMENT_HXX
#define _SDR_COMMENT_HXX

#include <sal/types.h>
#include <tools/date.hxx>
#include <rtl/ustring.hxx>
#include <basegfx/point/b2dpoint.hxx>

#include <vector>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    class Comment
    {
        // counting ID
        sal_uInt32                                      mnID;

        // creation date
        Date                                            maCreationDate;

        // name of creator
        ::rtl::OUString                                 maUserName;

        // text content
        ::rtl::OUString                                 maText;

        // position
        basegfx::B2DPoint                               maPosition;

    public:
        Comment(
            sal_uInt32 nID,
            Date aCreationDate,
            const ::rtl::OUString& rUserName,
            const ::rtl::OUString& rText,
            const basegfx::B2DPoint& rPosition);
        ~Comment();

        // operator for sorting the vector by mnID
        sal_Bool operator<(const Comment& rCandidate) const { return (mnID < rCandidate.mnID); }

        // comparison operators
        sal_Bool operator==(const Comment& rCandidate) const;
        sal_Bool operator!=(const Comment& rCandidate) const { return !(operator==(rCandidate)); }

        // access to ID, read only
        sal_uInt32 GetID() const { return mnID; }

        // access to CreationDate
        Date GetCreationDate() const { return maCreationDate; }
        void SetCreationDate(Date aNewDate);

        // access to UserName
        const ::rtl::OUString& GetUserName() const { return maUserName; }
        void SetUserName(const ::rtl::OUString& rNewName);

        // access to text
        const ::rtl::OUString& GetText() const { return maText; }
        void SetText(const ::rtl::OUString& rNewText);

        // access to position
        const basegfx::B2DPoint& GetPosition() const { return maPosition; }
        void SetPosition(const basegfx::B2DPoint& rNewPos);
    };

    // typedef for list of sdr::Comment
    typedef ::std::vector< Comment > CommentVector;
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_COMMENT_HXX
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
