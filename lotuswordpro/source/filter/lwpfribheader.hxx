/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _LWPFRIBHEADER_HXX
#define _LWPFRIBHEADER_HXX

#define FRIB_TAG_NOUNICODE 0x40 // Don't xlate text to/from Unicode
#define FRIB_TAG_MODIFIER 0x80  // This frib has a modifier on it
#define FRIB_TAG_TYPEMASK (FRIB_TAG_NOUNICODE | FRIB_TAG_MODIFIER)

#define FRIB_TAG_ELVIS          0       // "EOP dammit" tag

#define FRIB_MTAG_NONE          0
#define FRIB_MTAG_FONT          1
#define FRIB_MTAG_REVISION      2
#define FRIB_MTAG_CHARSTYLE     3
#define FRIB_MTAG_ATTRIBUTE     4
#define FRIB_MTAG_LANGUAGE      5
#define FRIB_MTAG_CODEPAGE      8

#define FT_MAXIMUM      30

#define DOC_BADFILEPARAHINTS 0x00800000UL

/* Registration definitions */
enum
{
    FS_RIGHTSIDE    = 0x00,
    FS_LEFTSIDE     = 0x10
};
/* Mode definitions */
enum
{
    FS_REGISTERSELF = 0x20,
    FS_READING      = 0x40
};

enum
{
    FRIB_TAG_INVALID,       // FT_INVALID
    FRIB_TAG_EOP,           // FT_EOP
    FRIB_TAG_TEXT,          // FT_TEXT
    FRIB_TAG_TABLE,         // FT_TABLE
    FRIB_TAG_TAB,           // FT_TAB
    FRIB_TAG_PAGEBREAK,     // FT_PAGEBREAK
    FRIB_TAG_FRAME,         // FT_FRAME
    FRIB_TAG_FOOTNOTE,      // FT_FOOTNOTE
    FRIB_TAG_COLBREAK,      // FT_COLBREAK
    FRIB_TAG_LINEBREAK,     // FT_LINEBREAK
    FRIB_TAG_HARDSPACE,     // FT_HARDSPACE
    FRIB_TAG_SOFTHYPHEN,    // FT_SOFTHYPHEN
    FRIB_TAG_PARANUMBER,    // FT_PARANUMBER
    FRIB_TAG_UNICODE,       // FT_UNICODE
#ifdef KANJI
    FRIB_TAG_KANJI,         // FT_KANJI
    FRIB_TAG_HKATAKANA,     // FT_HKATAKANA
#else
    FRIB_TAG_UNICODE2,      // FT_UNICODE
    FRIB_TAG_UNICODE3,      // FT_UNICODE
#endif
    FRIB_TAG_SEPARATOR,     // FT_SEPARATOR
    FRIB_TAG_SECTION,       // FT_SECTION
    FRIB_TAG_TOMBSTONE,     // FT_TOMBSTONE
    FRIB_TAG_SPECIALTAB,    // FT_SPECIALTAB
    FRIB_TAG_PAGENUMBER,    // FT_PAGENUMBER
    FRIB_TAG_NOTE,          // FT_NOTE
    FRIB_TAG_DOCVAR,        // FT_DOCVAR
    FRIB_TAG_BOOKMARK,      // FT_BOOKMARK
    FRIB_TAG_DDE,           // FT_DDE
    FRIB_TAG_FIELD,         // FT_FIELD
    FRIB_TAG_CHBLOCK,       // FT_CHBLOCK
//#ifdef RUBY
    FRIB_TAG_FLOWBREAK,     // FT_FLOWBREAK
    FRIB_TAG_RUBYMARKER,    // FT_RUBYMARKER
    FRIB_TAG_RUBYFRAME      // FT_RUBYFRAME
//#else
//  FRIB_TAG_FLOWBREAK      // FT_FLOWBREAK
//#endif
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
