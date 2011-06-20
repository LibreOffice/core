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

#include "fields.hxx"
#include <osl/diagnose.h>
#include <sal/macros.h>
#include <stddef.h>

namespace ww
{
    const char *GetEnglishFieldName(eField eIndex) throw()
    {
        //0 Signifies the field names I can't find.
        // #i43956# - field <eFOOTREF> = 5 should be mapped to "REF"
        static const char *aFieldNames[] =
        {
            /* 0*/  0,
            /* 1*/  0,
            /* 2*/  0,
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
            /*36*/  0,
            /*37*/  "PAGEREF",
            /*38*/  "ASK",
            /*39*/  "FILLIN",
            /*40*/  0,
            /*41*/  "NEXT",
            /*42*/  "NEXTIF",
            /*43*/  "SKIPIF",
            /*44*/  "MERGEREC",
            /*45*/  0,
            /*46*/  0,
            /*47*/  0,
            /*48*/  "PRINT",
            /*49*/  "EQ",
            /*50*/  "GOTOBUTTON",
            /*51*/  "MACROBUTTON",
            /*52*/  "AUTONUMOUT",
            /*53*/  "AUTONUMLGL",
            /*54*/  "AUTONUM",
            /*55*/  0,
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
            /*76*/  0,
            /*77*/  "PRIVATE",
            /*78*/  "DATABASE",
            /*79*/  "AUTOTEXT",
            /*80*/  "COMPARE",
            /*81*/  0,
            /*82*/  0,
            /*83*/  "FORMDROPDOWN",
            /*84*/  "ADVANCE",
            /*85*/  "DOCPROPERTY",
            /*86*/  0,
            /*87*/  "CONTROL",
            /*88*/  "HYPERLINK",
            /*89*/  "AUTOTEXTLIST",
            /*90*/  "LISTNUM",
            /*91*/  0,
            /*92*/  "BIDIOUTLINE",
            /*93*/  "ADDRESSBLOCK",
            /*94*/  "GREETINGLINE",
            /*95*/  "SHAPE"
        };

        size_t nIndex = static_cast<size_t>(eIndex);
        if (nIndex >= SAL_N_ELEMENTS(aFieldNames))
            eIndex = eNONE;
        OSL_ENSURE(eIndex != eNONE, "Unknown WinWord Field, let cmc know");
        return aFieldNames[eIndex];
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
