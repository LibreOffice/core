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

#include <sal/config.h>

#include <cstdlib>

#include <IDocumentMarkAccess.hxx>
#include <crossrefbookmark.hxx>
#include <ndtxt.hxx>

namespace
{
    const OUStringLiteral CrossRefNumItemBookmark_NamePrefix = u"__RefNumPara__";
}

namespace sw::mark
{
    CrossRefBookmark::CrossRefBookmark(const SwPaM& rPaM,
        const vcl::KeyCode& rCode,
        const OUString& rName,
        const OUString& rPrefix)
        : Bookmark(
                // ensure that m_pPos2 is null by only passing start to super
                SwPaM(*rPaM.Start()), rCode, rName)
    {
        assert( IDocumentMarkAccess::IsLegalPaMForCrossRefHeadingBookmark(rPaM)
                 && "<CrossRefBookmark::CrossRefBookmark(..)>"
                    "- creation of cross-reference bookmark with an illegal PaM that does not expand over exactly one whole paragraph.");
        if(rName.isEmpty())
            m_aName = MarkBase::GenerateNewName(rPrefix);
        assert(!m_pPos2);
    }

    void CrossRefBookmark::SetMarkPos(const SwPosition& rNewPos)
    {
        assert(rNewPos.nNode.GetNode().GetTextNode() &&
            "<sw::mark::CrossRefBookmark::SetMarkPos(..)>"
            " - new bookmark position for cross-reference bookmark doesn't mark text node");
        assert(rNewPos.nContent.GetIndex() == 0 &&
            "<sw::mark::CrossRefBookmark::SetMarkPos(..)>"
            " - new bookmark position for cross-reference bookmark doesn't mark start of text node");
        MarkBase::SetMarkPos(rNewPos);
    }

    SwPosition& CrossRefBookmark::GetOtherMarkPos() const
    {
        assert(false &&
            "<sw::mark::CrossRefBookmark::GetOtherMarkPos(..)>"
            " - this should never be called!");
        for (;;) { std::abort(); } // avoid "must return a value" warnings
    }

    CrossRefHeadingBookmark::CrossRefHeadingBookmark(const SwPaM& rPaM,
        const vcl::KeyCode& rCode,
        const OUString& rName)
        : CrossRefBookmark(rPaM, rCode, rName, IDocumentMarkAccess::GetCrossRefHeadingBookmarkNamePrefix()+"_Toc")
    { }

    bool CrossRefHeadingBookmark::IsLegalName(const OUString& rName)
    {
        return rName.match(IDocumentMarkAccess::GetCrossRefHeadingBookmarkNamePrefix());
    }

    CrossRefNumItemBookmark::CrossRefNumItemBookmark(const SwPaM& rPaM,
        const vcl::KeyCode& rCode,
        const OUString& rName)
        : CrossRefBookmark(rPaM, rCode, rName, CrossRefNumItemBookmark_NamePrefix)
    { }

    bool CrossRefNumItemBookmark::IsLegalName(const OUString& rName)
    {
        return rName.match(CrossRefNumItemBookmark_NamePrefix);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
