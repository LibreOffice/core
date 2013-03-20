/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <crossrefbookmark.hxx>
#include <ndtxt.hxx>

namespace
{

const char CrossRefHeadingBookmark_NamePrefix[] = "__RefHeading__";
const char CrossRefNumItemBookmark_NamePrefix[] = "__RefNumPara__";

}

namespace sw { namespace mark
{
    CrossRefBookmark::CrossRefBookmark(const SwPaM& rPaM,
        const KeyCode& rCode,
        const OUString& rName,
        const OUString& rShortName,
        const OUString& rPrefix)
        : Bookmark(rPaM, rCode, rName, rShortName)
    {
        if(rPaM.HasMark())
            OSL_ENSURE((rPaM.GetMark()->nNode == rPaM.GetPoint()->nNode &&
                rPaM.Start()->nContent.GetIndex() == 0 &&
                rPaM.End()->nContent.GetIndex() == rPaM.GetPoint()->nNode.GetNode().GetTxtNode()->Len()),
                "<CrossRefBookmark::CrossRefBookmark(..)>"
                "- creation of cross-reference bookmark with an expanded PaM that does not expand over exactly one whole paragraph.");
        SetMarkPos(*rPaM.Start());
        if(rName.isEmpty())
            m_aName = MarkBase::GenerateNewName(rPrefix);
    }

    void CrossRefBookmark::SetMarkPos(const SwPosition& rNewPos)
    {
        OSL_PRECOND(rNewPos.nNode.GetNode().GetTxtNode(),
            "<sw::mark::CrossRefBookmark::SetMarkPos(..)>"
            " - new bookmark position for cross-reference bookmark doesn't mark text node");
        OSL_PRECOND(rNewPos.nContent.GetIndex() == 0,
            "<sw::mark::CrossRefBookmark::SetMarkPos(..)>"
            " - new bookmark position for cross-reference bookmark doesn't mark start of text node");
        MarkBase::SetMarkPos(rNewPos);
    }

    SwPosition& CrossRefBookmark::GetOtherMarkPos() const
    {
        OSL_PRECOND(false,
            "<sw::mark::CrossRefBookmark::GetOtherMarkPos(..)>"
            " - this should never be called!");
        return *static_cast<SwPosition*>(NULL);
    }

    CrossRefHeadingBookmark::CrossRefHeadingBookmark(const SwPaM& rPaM,
        const KeyCode& rCode,
        const OUString& rName,
        const OUString& rShortName)
        : CrossRefBookmark(rPaM, rCode, rName, rShortName, OUString(CrossRefHeadingBookmark_NamePrefix))
    { }

    bool CrossRefHeadingBookmark::IsLegalName(const OUString& rName)
    {
        return rName.match(CrossRefHeadingBookmark_NamePrefix);
    }

    CrossRefNumItemBookmark::CrossRefNumItemBookmark(const SwPaM& rPaM,
        const KeyCode& rCode,
        const OUString& rName,
        const OUString& rShortName)
        : CrossRefBookmark(rPaM, rCode, rName, rShortName, OUString(CrossRefNumItemBookmark_NamePrefix))
    { }

    bool CrossRefNumItemBookmark::IsLegalName(const OUString& rName)
    {
        return rName.match(CrossRefNumItemBookmark_NamePrefix);
    }
}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
