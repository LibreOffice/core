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

#ifndef _TOKENS_H
#define _TOKENS_H

/*------------------------------------------------------                    */
/*------------------------------------------------------                    */
/* Tokens for parsing src files                                             */
/*------------------------------------------------------                    */
/*------------------------------------------------------                    */
#define IGNOREDTOKENS       400         /* #include | #pragma | //... | ... */
#define COMMEND             401         /*...                               */
#define DEFINEDRES          402         /* Text = {                         */
#define ANYTOKEN            404         /* XYZ                              */
#define UNKNOWNTOKEN        405         /* XYZ[ \t]$                        */
#define UNKNOWNCONSTRUCTION 406         /* XYZ ( xxx, yyy, zzz )            */
#define UNKNOWNCHAR         407         /* .                                */
/*------------------------------------------------------                    */
/* prev. tokens will not be executed                                        */
#define FILTER_LEVEL        500
/* following tokens will be executed                                        */
/*------------------------------------------------------                    */
#define CONDITION           501         /* #if... | #endif ... | ...        */
#define EMPTYLINE           502         /*                                  */
#define RESSOURCE           503         /* Menu MID_TEST                    */
#define RESSOURCEEXPR       504         /* Menu ( MID_TEST + .. )           */
#define SMALRESSOURCE       505         /* PageItem {                       */
#define TEXTLINE            506         /* TEXT = "hhh"                     */
#define LONGTEXTLINE        507         /* TEXT = "hhh" TEST "HHH" ...      */
#define TEXT                508         /* "Something like this"            */
#define LEVELUP             509         /* {                                */
#define LEVELDOWN           510         /* };                               */
#define APPFONTMAPPING      511         /* MAP_APPFONT(10,10)               */
#define ASSIGNMENT          512         /* Somathing = Anything             */
#define LISTASSIGNMENT      513         /* ...List [xyz]=...                */
#define LISTTEXT            514         /* < "Text" ... >                   */
#define RSCDEFINE           515         /* #define MY_TEXT                  */
#define RSCDEFINELEND       516         /*                                  */
#define NEWTEXTINRES        517         /* ### Achtung : Ne...              */
#define UIENTRIES           518         /* UIEntries = {                    */
#define PRAGMA              519         /* #pragma ...                      */
#define _LISTTEXT           521         /* { "Text" ... }                   */
#define TEXTREFID           522         /* Text = 12345                     */
#define LISTRESID           523         /* < 12345; ... >                   */
#define _LISTRESID          523         /* { 12345; ... }                   */
#define NORMDEFINE          524         /* #define ...                      */
/*------------------------------------------------------                    */
/*------------------------------------------------------                    */
/* Tokens for parsing cfg files                                             */
/*------------------------------------------------------                    */
/*------------------------------------------------------                    */
#define CFG_TAG                     501
#define CFG_TEXT_START              505
#define CFG_TEXT_END                506
#define CFG_TEXTCHAR                507
#define CFG_CLOSETAG                508
#define CFG_UNKNOWNTAG              509
#define CFG_TOKEN_PACKAGE           600
#define CFG_TOKEN_COMPONENT         601
#define CFG_TOKEN_CONFIGNAME        602
#define CFG_TOKEN_TEMPLATE          603
#define CFG_TOKEN_OORNAME           604
#define CFG_TOKEN_OORVALUE          605
#define CFG_TOKEN_NO_TRANSLATE      606

/*------------------------------------------------------                    */
/*------------------------------------------------------                    */
/* Tokens for parsing xrm files                                             */
/*------------------------------------------------------                    */
/*------------------------------------------------------                    */
#define XRM_README_START            501
#define XRM_README_END              502
#define XRM_SECTION_START           503
#define XRM_SECTION_END             504
#define XRM_PARAGRAPH_START         505
#define XRM_PARAGRAPH_END           506
#define XRM_TEXT_START              507
#define XRM_TEXT_END                508
#define XRM_LIST_START              509
#define XRM_LIST_END                510
#define XML_TEXTCHAR                600


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
