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

#pragma once

#include "scitems.hxx"

//  WIDs for uno property maps,
//  never stored in files

#define SC_WID_UNO_START    1200

#define SC_WID_UNO_CELLSTYL         ( SC_WID_UNO_START + 0 )
#define SC_WID_UNO_CHCOLHDR         ( SC_WID_UNO_START + 1 )
#define SC_WID_UNO_CHROWHDR         ( SC_WID_UNO_START + 2 )
#define SC_WID_UNO_CONDFMT          ( SC_WID_UNO_START + 3 )
#define SC_WID_UNO_CONDLOC          ( SC_WID_UNO_START + 4 )
#define SC_WID_UNO_CONDXML          ( SC_WID_UNO_START + 5 )
#define SC_WID_UNO_TBLBORD          ( SC_WID_UNO_START + 6 )
#define SC_WID_UNO_VALIDAT          ( SC_WID_UNO_START + 7 )
#define SC_WID_UNO_VALILOC          ( SC_WID_UNO_START + 8 )
#define SC_WID_UNO_VALIXML          ( SC_WID_UNO_START + 9 )
#define SC_WID_UNO_POS              ( SC_WID_UNO_START + 10 )
#define SC_WID_UNO_SIZE             ( SC_WID_UNO_START + 11 )
#define SC_WID_UNO_FORMLOC          ( SC_WID_UNO_START + 12 )
#define SC_WID_UNO_FORMRT           ( SC_WID_UNO_START + 13 )
#define SC_WID_UNO_PAGESTL          ( SC_WID_UNO_START + 14 )
#define SC_WID_UNO_CELLVIS          ( SC_WID_UNO_START + 15 )
#define SC_WID_UNO_LINKDISPBIT      ( SC_WID_UNO_START + 16 )
#define SC_WID_UNO_LINKDISPNAME     ( SC_WID_UNO_START + 17 )
#define SC_WID_UNO_CELLWID          ( SC_WID_UNO_START + 18 )
#define SC_WID_UNO_OWIDTH           ( SC_WID_UNO_START + 19 )
#define SC_WID_UNO_NEWPAGE          ( SC_WID_UNO_START + 20 )
#define SC_WID_UNO_MANPAGE          ( SC_WID_UNO_START + 21 )
#define SC_WID_UNO_CELLHGT          ( SC_WID_UNO_START + 22 )
#define SC_WID_UNO_CELLFILT         ( SC_WID_UNO_START + 23 )
#define SC_WID_UNO_OHEIGHT          ( SC_WID_UNO_START + 24 )
#define SC_WID_UNO_DISPNAME         ( SC_WID_UNO_START + 25 )
#define SC_WID_UNO_HEADERSET        ( SC_WID_UNO_START + 26 )
#define SC_WID_UNO_FOOTERSET        ( SC_WID_UNO_START + 27 )
#define SC_WID_UNO_NUMRULES         ( SC_WID_UNO_START + 28 )
#define SC_WID_UNO_ISACTIVE         ( SC_WID_UNO_START + 29 )
#define SC_WID_UNO_BORDCOL          ( SC_WID_UNO_START + 30 )
#define SC_WID_UNO_PROTECT          ( SC_WID_UNO_START + 31 )
#define SC_WID_UNO_SHOWBORD         ( SC_WID_UNO_START + 32 )
#define SC_WID_UNO_PRINTBORD        ( SC_WID_UNO_START + 33 )
#define SC_WID_UNO_COPYBACK         ( SC_WID_UNO_START + 34 )
#define SC_WID_UNO_COPYSTYL         ( SC_WID_UNO_START + 35 )
#define SC_WID_UNO_COPYFORM         ( SC_WID_UNO_START + 36 )
#define SC_WID_UNO_TABLAYOUT        ( SC_WID_UNO_START + 37 )
#define SC_WID_UNO_AUTOPRINT        ( SC_WID_UNO_START + 38 )
#define SC_WID_UNO_ABSNAME          ( SC_WID_UNO_START + 39 )
#define SC_WID_UNO_CODENAME         ( SC_WID_UNO_START + 40 )
#define SC_WID_UNO_TABCOLOR         ( SC_WID_UNO_START + 41 )
#define SC_WID_UNO_NAMES            ( SC_WID_UNO_START + 42 )
#define SC_WID_UNO_TBLBORD2         ( SC_WID_UNO_START + 43 )
#define SC_WID_UNO_CONDFORMAT       ( SC_WID_UNO_START + 44 )
#define SC_WID_UNO_FORMATID         ( SC_WID_UNO_START + 45 )
#define SC_WID_UNO_FORMRT2          ( SC_WID_UNO_START + 46 )
#define SC_WID_UNO_CELLCONTENTTYPE  ( SC_WID_UNO_START + 47 )
#define SC_WID_UNO_END              ( SC_WID_UNO_START + 47 )

inline bool IsScUnoWid( sal_uInt16 nWid )
{
    return nWid >= SC_WID_UNO_START && nWid <= SC_WID_UNO_END;
}

inline bool IsScItemWid( sal_uInt16 nWid )
{
    return nWid >= ATTR_STARTINDEX && nWid <= ATTR_ENDINDEX;    // incl. page
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
