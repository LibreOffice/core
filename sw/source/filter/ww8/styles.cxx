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
        static const char *stiName[] =
        {
            "Normal",                   // stiNormal
            "Heading 1",                // stiLev1
            "Heading 2",                // stiLev2
            "Heading 3",                // stiLev3
            "Heading 4",                // stiLev4
            "Heading 5",                // stiLev5
            "Heading 6",                // stiLev6
            "Heading 7",                // stiLev7
            "Heading 8",                // stiLev8
            "Heading 9",                // stiLev9
            "Index 1",                  // stiIndex1
            "Index 2",                  // stiIndex2
            "Index 3",                  // stiIndex3
            "Index 4",                  // stiIndex4
            "Index 5",                  // stiIndex5
            "Index 6",                  // stiIndex6
            "Index 7",                  // stiIndex7
            "Index 8",                  // stiIndex8
            "Index 9",                  // stiIndex9
            "TOC 1",                    // stiToc1
            "TOC 2",                    // stiToc2
            "TOC 3",                    // stiToc3
            "TOC 4",                    // stiToc4
            "TOC 5",                    // stiToc5
            "TOC 6",                    // stiToc6
            "TOC 7",                    // stiToc7
            "TOC 8",                    // stiToc8
            "TOC 9",                    // stiToc9
            "Normal Indent",            // stiNormIndent
            "Footnote Text",            // stiFootnoteText
            "Annotation Text",          // stiAtnText
            "Header",                   // stiHeader
            "Footer",                   // stiFooter
            "Index Heading",            // stiIndexHeading
            "Caption",                  // stiCaption
            "Table of Figures",         // stiToCaption
            "Envelope Address",         // stiEnvAddr
            "Envelope Return",          // stiEnvRet
            "Footnote Reference",       // stiFootnoteRef
            "Annotation Reference",     // stiAtnRef
            "Line Number",              // stiLnn
            "Page Number",              // stiPgn
            "Endnote Reference",        // stiEdnRef
            "Endnote Text",             // stiEdnText
            "Table of Authorities",     // stiToa
            "Macro Text",               // stiMacro
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
