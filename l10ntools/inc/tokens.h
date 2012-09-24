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

#ifndef _TOKENS_H
#define _TOKENS_H

/*------------------------------------------------------                    */
/*------------------------------------------------------                    */
/* Tokens for parsing src files                                             */
/*------------------------------------------------------                    */
/*------------------------------------------------------                    */
#define IGNOREDTOKENS       400         /* #include | #pragma | //... | ... */
#define COMMENT             401         /*...                               */
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
#define RESOURCE            503         /* Menu MID_TEST                    */
#define RESOURCEEXPR        504         /* Menu ( MID_TEST + .. )           */
#define SMALRESOURCE        505         /* PageItem {                       */
#define TEXTLINE            506         /* TEXT = "hhh"                     */
#define LONGTEXTLINE        507         /* TEXT = "hhh" TEST "HHH" ...      */
#define TEXT                508         /* "Something like this"            */
#define LEVELUP             509         /* {                                */
#define LEVELDOWN           510         /* };                               */
#define APPFONTMAPPING      511         /* MAP_APPFONT(10,10)               */
#define ASSIGNMENT          512         /* Something = Anything             */
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
#define XRM_TEXT_START              507
#define XRM_TEXT_END                508
#define XML_TEXTCHAR                600

/*------------------------------------------------------                    */
/*------------------------------------------------------                    */
/* Tokens for parsing description.xml files                                 */
/*------------------------------------------------------                    */
/*------------------------------------------------------                    */
#define DESC_DISPLAY_NAME_START          700
#define DESC_DISPLAY_NAME_END            701
#define DESC_TEXT_START                  702
#define DESC_TEXT_END                    703
#define DESC_EXTENSION_DESCRIPTION_START 704
#define DESC_EXTENSION_DESCRIPTION_END   705
#define DESC_EXTENSION_DESCRIPTION_SRC   706

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
