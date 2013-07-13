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


#include "../inc/wwstyles.hxx"

#include <functional>               //std::unary_function
#include <algorithm>                //std::find_if
#include <tools/string.hxx>         //do we have to...

#include "staticassert.hxx"      //StaticAssert

namespace
{
    class SameName: public std::unary_function<const sal_Char*, bool>
    {
    private:
        const String &mrName;
    public:
        explicit SameName(const String &rName) : mrName(rName) {}
        bool operator() (const sal_Char *pEntry) const
            { return mrName.EqualsAscii(pEntry); }
    };

    const sal_Char **GetStiNames() throw()
    {
        static const sal_Char *stiName[] =
        {
            "Normal",
            "Heading 1",
            "Heading 2",
            "Heading 3",
            "Heading 4",
            "Heading 5",
            "Heading 6",
            "Heading 7",
            "Heading 8",
            "Heading 9",
            "Index 1",
            "Index 2",
            "Index 3",
            "Index 4",
            "Index 5",
            "Index 6",
            "Index 7",
            "Index 8",
            "Index 9",
            "TOC 1",
            "TOC 2",
            "TOC 3",
            "TOC 4",
            "TOC 5",
            "TOC 6",
            "TOC 7",
            "TOC 8",
            "TOC 9",
            "Normal Indent",
            "Footnote Text",
            "Annotation Text",
            "Header",
            "Footer",
            "Index Heading",
            "Caption",
            "Table of Figures",
            "Envelope Address",
            "Envelope Return",
            "Footnote Reference",
            "Annotation Reference",
            "Line Number",
            "Page Number",
            "Endnote Reference",
            "Endnote Text",
            "Table of Authorities",
            "Macro Text",
            "TOA Heading",
            "List",
            "List 2",
            "List 3",
            "List 4",
            "List 5",
            "List Bullet",
            "List Bullet 2",
            "List Bullet 3",
            "List Bullet 4",
            "List Bullet 5",
            "List Number",
            "List Number 2",
            "List Number 3",
            "List Number 4",
            "List Number 5",
            "Title",
            "Closing",
            "Signature",
            "Default Paragraph Font",
            "Body Text",
            "Body Text Indent",
            "List Continue",
            "List Continue 2",
            "List Continue 3",
            "List Continue 4",
            "List Continue 5",
            "Message Header",
            "Subtitle",
            "Salutation",
            "Date",
            "Body Text First Indent",
            "Body Text First Indent 2",
            "Note Heading",
            "Body Text 2",
            "Body Text 3",
            "Body Text Indent 2",
            "Body Text Indent 3",
            "Block Text",
            "Hyperlink",
            "Followed Hyperlink",
            "Strong",
            "Emphasis",
            "Document Map",
            "Plain Text"
        };

        OSL_ENSURE( (sizeof (stiName) / sizeof (stiName[0])) == ww::stiMax, "WrongSizeOfArray" );

        return stiName;
    }
}

namespace ww
{
    const sal_Char* GetEnglishNameFromSti(sti eSti) throw()
    {
        if (eSti >= stiMax)
            return 0;
        else
            return GetStiNames()[eSti];
    }

    bool StandardStiIsCharStyle(sti eSti) throw()
    {
        switch (eSti)
        {
            case stiFtnRef:
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

    sti GetCanonicalStiFromStc(sal_uInt8 stc) throw()
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
                stiFtnRef, stiFtnText, stiLev9, stiLev8, stiLev7, stiLev6,
                stiLev5, stiLev4, stiLev3, stiLev2, stiLev1, stiNormIndent
            };
            return aMapping[stc-222];
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
