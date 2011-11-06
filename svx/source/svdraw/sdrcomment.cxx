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
