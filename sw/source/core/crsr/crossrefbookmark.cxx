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
#include "precompiled_sw.hxx"

#include <crossrefbookmark.hxx>
#include <ndtxt.hxx>

using ::rtl::OUString;
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

    SwPosition& CrossRefBookmark::GetOtherMarkPos() const
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
        : CrossRefBookmark(rPaM, rCode, rName, rShortName, our_sNamePrefix)
    { }

    const ::rtl::OUString CrossRefHeadingBookmark::our_sNamePrefix(RTL_CONSTASCII_USTRINGPARAM("__RefHeading__"));

    bool CrossRefHeadingBookmark::IsLegalName(const ::rtl::OUString& rName)
    {
        return rName.match(our_sNamePrefix);
    }

    CrossRefNumItemBookmark::CrossRefNumItemBookmark(const SwPaM& rPaM,
        const KeyCode& rCode,
        const OUString& rName,
        const OUString& rShortName)
        : CrossRefBookmark(rPaM, rCode, rName, rShortName, our_sNamePrefix)
    { }

    const ::rtl::OUString CrossRefNumItemBookmark::our_sNamePrefix(RTL_CONSTASCII_USTRINGPARAM("__RefNumPara__"));

    bool CrossRefNumItemBookmark::IsLegalName(const ::rtl::OUString& rName)
    {
        return rName.match(our_sNamePrefix);
    }
}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
