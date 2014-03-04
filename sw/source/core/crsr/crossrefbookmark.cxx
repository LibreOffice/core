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
#include "precompiled_sw.hxx"

#include <IDocumentMarkAccess.hxx>
#include <crossrefbookmark.hxx>
#include <ndtxt.hxx>

using namespace rtl;

namespace sw { namespace mark
{
    CrossRefBookmark::CrossRefBookmark(const SwPaM& rPaM,
        const KeyCode& rCode,
        const OUString& rName,
        const OUString& rShortName,
        const OUString& rPrefix)
        : Bookmark(rPaM, rCode, rName, rShortName)
    {
        OSL_ENSURE( IDocumentMarkAccess::IsLegalPaMForCrossRefHeadingBookmark( rPaM ),
                    "<CrossRefBookmark::CrossRefBookmark(..)>"
                    "- creation of cross-reference bookmark with an illegal PaM that does not expand over exactly one whole paragraph.");
        SetMarkPos(*rPaM.Start());
        if(!rName.getLength())
            m_aName = MarkBase::GenerateNewName(rPrefix);
    }

    void CrossRefBookmark::SetMarkPos(const SwPosition& rNewPos)
    {
        OSL_PRECOND(rNewPos.nNode.GetNode().GetTxtNode(),
            "<SwCrossRefBookmark::SetMarkPos(..)>"
            " - new bookmark position for cross-reference bookmark doesn't mark text node");
        OSL_PRECOND(rNewPos.nContent.GetIndex() == 0,
            "<SwCrossRefBookmark::SetMarkPos(..)>"
            " - new bookmark position for cross-reference bookmark doesn't mark start of text node");
        MarkBase::SetMarkPos(rNewPos);
    }

    const SwPosition& CrossRefBookmark::GetOtherMarkPos() const
    {
        OSL_PRECOND(false,
            "<SwCrossRefBookmark::GetOtherMarkPos(..)>"
            " - this should never be called!");
        return *static_cast<SwPosition*>(NULL);
    }

    CrossRefHeadingBookmark::CrossRefHeadingBookmark(const SwPaM& rPaM,
        const KeyCode& rCode,
        const OUString& rName,
        const OUString& rShortName)
        : CrossRefBookmark(rPaM, rCode, rName, rShortName, IDocumentMarkAccess::GetCrossRefHeadingBookmarkNamePrefix())
    { }

    bool CrossRefHeadingBookmark::IsLegalName(const ::rtl::OUString& rName)
    {
        return rName.match(IDocumentMarkAccess::GetCrossRefHeadingBookmarkNamePrefix());
    }

    CrossRefNumItemBookmark::CrossRefNumItemBookmark(const SwPaM& rPaM,
        const KeyCode& rCode,
        const OUString& rName,
        const OUString& rShortName)
        : CrossRefBookmark(rPaM, rCode, rName, rShortName, our_sNamePrefix)
    { }

    const ::rtl::OUString CrossRefNumItemBookmark::our_sNamePrefix = ::rtl::OUString::createFromAscii("__RefNumPara__");

    bool CrossRefNumItemBookmark::IsLegalName(const ::rtl::OUString& rName)
    {
        return rName.match(our_sNamePrefix);
    }
}}
