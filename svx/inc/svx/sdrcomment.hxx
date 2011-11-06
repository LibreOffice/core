/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
