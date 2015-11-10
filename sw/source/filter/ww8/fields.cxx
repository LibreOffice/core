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

#include "fields.hxx"
#include <osl/diagnose.h>
#include <stddef.h>

namespace ww
{
    const char *GetEnglishFieldName(eField eIndex) throw()
    {
        //0 Signifies the field names I can't find.
        // #i43956# - field <eFOOTREF> = 5 should be mapped to "REF"
        static const char *aFieldNames[] =
        {
            /* 0*/  nullptr,
            /* 1*/  nullptr,
            /* 2*/  nullptr,
            /* 3*/  "REF",
            /* 4*/  "XE",
            /* 5*/  "REF",
            /* 6*/  "SET",
            /* 7*/  "IF",
            /* 8*/  "INDEX",
            /* 9*/  "TC",
            /*10*/  "STYLEREF",
            /*11*/  "RD",
            /*12*/  "SEQ",
            /*13*/  "TOC",
            /*14*/  "INFO",
            /*15*/  "TITLE",
            /*16*/  "SUBJECT",
            /*17*/  "AUTHOR",
            /*18*/  "KEYWORDS",
            /*19*/  "COMMENTS",
            /*20*/  "LASTSAVEDBY",
            /*21*/  "CREATEDATE",
            /*22*/  "SAVEDATE",
            /*23*/  "PRINTDATE",
            /*24*/  "REVNUM",
            /*25*/  "EDITTIME",
            /*26*/  "NUMPAGES",
            /*27*/  "NUMWORDS",
            /*28*/  "NUMCHARS",
            /*29*/  "FILENAME",
            /*30*/  "TEMPLATE",
            /*31*/  "DATE",
            /*32*/  "TIME",
            /*33*/  "PAGE",
            /*34*/  "=",
            /*35*/  "QUOTE",
            /*36*/  nullptr,
            /*37*/  "PAGEREF",
            /*38*/  "ASK",
            /*39*/  "FILLIN",
            /*40*/  nullptr,
            /*41*/  "NEXT",
            /*42*/  "NEXTIF",
            /*43*/  "SKIPIF",
            /*44*/  "MERGEREC",
            /*45*/  nullptr,
            /*46*/  nullptr,
            /*47*/  nullptr,
            /*48*/  "PRINT",
            /*49*/  "EQ",
            /*50*/  "GOTOBUTTON",
            /*51*/  "MACROBUTTON",
            /*52*/  "AUTONUMOUT",
            /*53*/  "AUTONUMLGL",
            /*54*/  "AUTONUM",
            /*55*/  nullptr,
            /*56*/  "LINK",
            /*57*/  "SYMBOL",
            /*58*/  "EMBED",
            /*59*/  "MERGEFIELD",
            /*60*/  "USERNAME",
            /*61*/  "USERINITIALS",
            /*62*/  "USERADDRESS",
            /*63*/  "BARCODE",
            /*64*/  "DOCVARIABLE",
            /*65*/  "SECTION",
            /*66*/  "SECTIONPAGES",
            /*67*/  "INCLUDEPICTURE",
            /*68*/  "INCLUDETEXT",
            /*69*/  "FILESIZE",
            /*70*/  "FORMTEXT",
            /*71*/  "FORMCHECKBOX",
            /*72*/  "NOTEREF",
            /*73*/  "TOA",
            /*74*/  "TA",
            /*75*/  "MERGESEQ",
            /*76*/  nullptr,
            /*77*/  "PRIVATE",
            /*78*/  "DATABASE",
            /*79*/  "AUTOTEXT",
            /*80*/  "COMPARE",
            /*81*/  nullptr,
            /*82*/  nullptr,
            /*83*/  "FORMDROPDOWN",
            /*84*/  "ADVANCE",
            /*85*/  "DOCPROPERTY",
            /*86*/  nullptr,
            /*87*/  "CONTROL",
            /*88*/  "HYPERLINK",
            /*89*/  "AUTOTEXTLIST",
            /*90*/  "LISTNUM",
            /*91*/  nullptr,
            /*92*/  "BIDIOUTLINE",
            /*93*/  "ADDRESSBLOCK",
            /*94*/  "GREETINGLINE",
            /*95*/  "SHAPE",
            /*96*/  "BIBLIOGRAPHY",
            /*97*/  "CITATION"
        };

        size_t nIndex = static_cast<size_t>(eIndex);
        if (nIndex >= sizeof(aFieldNames) / sizeof(aFieldNames[0]))
            eIndex = eNONE;
        OSL_ENSURE(eIndex != eNONE, "Unknown WinWord Field");
        return aFieldNames[eIndex];
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
