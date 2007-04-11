/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdrcomment.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:10:35 $
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

#ifndef _SDR_COMMENT_HXX
#define _SDR_COMMENT_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _DATE_HXX
#include <tools/date.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
#endif

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
