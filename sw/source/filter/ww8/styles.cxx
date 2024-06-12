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

#include <wwstyles.hxx>

#include <osl/diagnose.h>

namespace
{
    // Keep in sync with StyleSheetTable::ConvertStyleName
    const char **GetStiNames() noexcept
    {
        // Matches enum ww::sti in sw/source/filter/inc/wwstyles.hxx
        // tdf#161509: The case of the names must match Word-generated document's w:latentStyles
        static const char *stiName[] =
        {
            "Normal",                   // stiNormal
            "heading 1",                // stiLev1
            "heading 2",                // stiLev2
            "heading 3",                // stiLev3
            "heading 4",                // stiLev4
            "heading 5",                // stiLev5
            "heading 6",                // stiLev6
            "heading 7",                // stiLev7
            "heading 8",                // stiLev8
            "heading 9",                // stiLev9
            "index 1",                  // stiIndex1
            "index 2",                  // stiIndex2
            "index 3",                  // stiIndex3
            "index 4",                  // stiIndex4
            "index 5",                  // stiIndex5
            "index 6",                  // stiIndex6
            "index 7",                  // stiIndex7
            "index 8",                  // stiIndex8
            "index 9",                  // stiIndex9
            "toc 1",                    // stiToc1
            "toc 2",                    // stiToc2
            "toc 3",                    // stiToc3
            "toc 4",                    // stiToc4
            "toc 5",                    // stiToc5
            "toc 6",                    // stiToc6
            "toc 7",                    // stiToc7
            "toc 8",                    // stiToc8
            "toc 9",                    // stiToc9
            "Normal Indent",            // stiNormIndent
            "footnote text",            // stiFootnoteText
            "annotation text",          // stiAtnText
            "header",                   // stiHeader
            "footer",                   // stiFooter
            "index heading",            // stiIndexHeading
            "caption",                  // stiCaption
            "table of figures",         // stiToCaption
            "envelope address",         // stiEnvAddr
            "envelope return",          // stiEnvRet
            "footnote reference",       // stiFootnoteRef
            "annotation reference",     // stiAtnRef
            "line number",              // stiLnn
            "page number",              // stiPgn
            "endnote reference",        // stiEdnRef
            "endnote text",             // stiEdnText
            "table of authorities",     // stiToa
            "macro",                    // stiMacro
            "TOC Heading",              // stiToaHeading - tdf143726
            "List",                     // stiList
            "List Bullet",              // stiListBullet
            "List Number",              // stiListNumber
            "List 2",                   // stiList2
            "List 3",                   // stiList3
            "List 4",                   // stiList4
            "List 5",                   // stiList5
            "List Bullet 2",            // stiListBullet2
            "List Bullet 3",            // stiListBullet3
            "List Bullet 4",            // stiListBullet4
            "List Bullet 5",            // stiListBullet5
            "List Number 2",            // stiListNumber2
            "List Number 3",            // stiListNumber3
            "List Number 4",            // stiListNumber4
            "List Number 5",            // stiListNumber5
            "Title",                    // stiTitle
            "Closing",                  // stiClosing
            "Signature",                // stiSignature
            "Default Paragraph Font",   // stiNormalChar
            "Body Text",                // stiBodyText
            "Body Text Indent",         // stiBodyTextInd1
            "List Continue",            // stiListCont
            "List Continue 2",          // stiListCont2
            "List Continue 3",          // stiListCont3
            "List Continue 4",          // stiListCont4
            "List Continue 5",          // stiListCont5
            "Message Header",           // stiMsgHeader
            "Subtitle",                 // stiSubtitle
            "Salutation",               // stiSalutation
            "Date",                     // stiDate
            "Body Text First Indent",   // stiBodyText1I
            "Body Text First Indent 2", // stiBodyText1I2
            "Note Heading",             // stiNoteHeading
            "Body Text 2",              // stiBodyText2
            "Body Text 3",              // stiBodyText3
            "Body Text Indent 2",       // stiBodyTextInd2
            "Body Text Indent 3",       // stiBodyTextInd3
            "Block Text",               // stiBlockQuote
            "Hyperlink",                // stiHyperlink
            "FollowedHyperlink",        // stiHyperlinkFollowed
            "Strong",                   // stiStrong
            "Emphasis",                 // stiEmphasis
            "Document Map",             // stiNavPane
            "Plain Text",               // stiPlainText
        };

        static_assert(SAL_N_ELEMENTS(stiName) == ww::stiMax, "WrongSizeOfArray");

        return stiName;
    }
}

namespace ww
{
    const char* GetEnglishNameFromSti(sti eSti) noexcept
    {
        if (eSti >= stiMax)
            return nullptr;
        else
            return GetStiNames()[eSti];
    }

    bool StandardStiIsCharStyle(sti eSti) noexcept
    {
        switch (eSti)
        {
            case stiFootnoteRef:
            case stiAtnRef:
            case stiLnn:
            case stiPgn:
            case stiEdnRef:
            case stiNormalChar:
                return true;
            default:
                return false;
        }
    }

    sti GetCanonicalStiFromStc(sal_uInt8 stc) noexcept
    {
        if (stc == 0)
            return stiNormal;
        else if (stc < 222)
            return stiUser;
        else
        {
            static const sti aMapping[] =
            {
                stiNil, stiAtnRef, stiAtnText, stiToc8, stiToc7, stiToc6,
                stiToc5, stiToc4, stiToc3, stiToc2, stiToc1, stiIndex7,
                stiIndex6, stiIndex5, stiIndex4, stiIndex3, stiIndex2,
                stiIndex1, stiLnn, stiIndexHeading, stiFooter, stiHeader,
                stiFootnoteRef, stiFootnoteText, stiLev9, stiLev8, stiLev7, stiLev6,
                stiLev5, stiLev4, stiLev3, stiLev2, stiLev1, stiNormIndent
            };
            return aMapping[stc-222];
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
