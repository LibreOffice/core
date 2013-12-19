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


#ifndef _CROSSREFBOOKMRK_HXX
#define _CROSSREFBOOKMRK_HXX

#include <IMark.hxx>
#include <bookmrk.hxx>
#include <rtl/ustring.hxx>


namespace sw { namespace mark
{
    class CrossRefBookmark
        : public Bookmark
    {
        public:
            CrossRefBookmark(const SwPaM& rPaM,
                const KeyCode& rCode,
                const ::rtl::OUString& rName,
                const ::rtl::OUString& rShortName,
                const ::rtl::OUString& rPrefix);

            // getters
            virtual SwPosition& GetOtherMarkPos() const;
            virtual SwPosition& GetMarkStart() const
                { return *m_pPos1; }
            virtual SwPosition& GetMarkEnd() const
                { return *m_pPos1; }
            virtual bool IsExpanded() const
                { return false; }

            // setters
            virtual void SetMarkPos(const SwPosition& rNewPos);
            virtual void SetOtherMarkPos(const SwPosition&)
            {
                OSL_PRECOND(false,
                    "<CrossRefBookmark::SetOtherMarkPos(..)>"
                    " - misusage of CrossRefBookmark: other bookmark position isn't allowed to be set." );
            }
            virtual void ClearOtherMarkPos()
            {
                OSL_PRECOND(false,
                    "<SwCrossRefBookmark::ClearOtherMarkPos(..)>"
                    " - misusage of CrossRefBookmark: other bookmark position isn't allowed to be set or cleared." );
            }
    };

    class CrossRefHeadingBookmark
        : public CrossRefBookmark
    {
        public:
            CrossRefHeadingBookmark(const SwPaM& rPaM,
                const KeyCode& rCode,
                const ::rtl::OUString& rName,
                const ::rtl::OUString& rShortName);
            static bool IsLegalName(const ::rtl::OUString& rName);
    };

    class CrossRefNumItemBookmark
        : public CrossRefBookmark
    {
        public:
            CrossRefNumItemBookmark(const SwPaM& rPaM,
                const KeyCode& rCode,
                const ::rtl::OUString& rName,
                const ::rtl::OUString& rShortName);
            static bool IsLegalName(const ::rtl::OUString& rName);
            static const ::rtl::OUString our_sNamePrefix;
    };

}}
#endif
