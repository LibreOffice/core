/*************************************************************************
 *
 *  $RCSfile: fields.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-25 07:40:18 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

#ifndef WW_FIELDS_HXX
#include "fields.hxx"
#endif

#ifndef _ERRHDL_HXX
#include <errhdl.hxx>   //ASSERT (use our own ww header later for asserts)
#endif

namespace ww
{
    const char *GetEnglishFieldName(eField eIndex) throw()
    {
        //0 Signifies the field names I can't find.
        static const char *aFieldNames[] =
        {
            /* 0*/  0,
            /* 1*/  0,
            /* 2*/  0,
            /* 3*/  "REF",
            /* 4*/  "XE",
            /* 5*/  0,
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
            /*26*/  "NUMPAGE",
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

        if (eIndex >= sizeof(aFieldNames) / sizeof(aFieldNames[0]))
            eIndex = eNONE;
        ASSERT(eIndex != eNONE, "Unknown WinWord Field, let cmc know");
        return aFieldNames[eIndex];
    }
}

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
