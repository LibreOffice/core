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

#ifndef _XDEF_HXX
#define _XDEF_HXX


/*************************************************************************
|*
|* Adding an attribute: Copy line, above which you want to add something,
|* change the label and add it at the end of the following line, too
|*
\************************************************************************/

#define COL_DEFAULT_SHAPE_FILLING  RGB_COLORDATA( 0x72, 0x9F, 0xCF )
#define COL_DEFAULT_SHAPE_STROKE    RGB_COLORDATA( 52, 101, 175 )

#define XATTR_START             1000

#define XATTR_LINE_FIRST        XATTR_START                     /* V3: 1000  V2: 1000 */
#define XATTR_LINESTYLE         XATTR_LINE_FIRST
#define XATTR_LINEDASH          (XATTR_LINE_FIRST + 1)          /* V3: 1001  V2: 1001 */
#define XATTR_LINEWIDTH         (XATTR_LINE_FIRST + 2)          /* V3: 1002  V2: 1002 */
#define XATTR_LINECOLOR         (XATTR_LINE_FIRST + 3)          /* V3: 1003  V2: 1003 */
#define XATTR_LINESTART         (XATTR_LINE_FIRST + 4)          /* V3: 1004  V2: 1004 */
#define XATTR_LINEEND           (XATTR_LINE_FIRST + 5)          /* V3: 1005  V2: 1005 */
#define XATTR_LINESTARTWIDTH    (XATTR_LINE_FIRST + 6)          /* V3: 1006  V2: 1006 */
#define XATTR_LINEENDWIDTH      (XATTR_LINE_FIRST + 7)          /* V3: 1007  V2: 1007 */
#define XATTR_LINESTARTCENTER   (XATTR_LINE_FIRST + 8)          /* V3: 1008  V2: 1008 */
#define XATTR_LINEENDCENTER     (XATTR_LINE_FIRST + 9)          /* V3: 1009  V2: 1009 */
#define XATTR_LINETRANSPARENCE  (XATTR_LINE_FIRST + 10)         /* V3: 1010  V2: 1010 */
#define XATTR_LINEJOINT         (XATTR_LINE_FIRST + 11)         /* V3: 1011  V2: 1011 */
#define XATTR_LINECAP           (XATTR_LINE_FIRST + 12)         /* V3: 1012 */
#define XATTR_LINE_LAST         XATTR_LINECAP
#define XATTRSET_LINE           (XATTR_LINE_LAST + 1)           /* V3: 1017  V2: 1017 */

#define XATTR_FILL_FIRST            (XATTRSET_LINE + 1)         /* V3: 1018  V2: 1018 */
#define XATTR_FILLSTYLE             XATTR_FILL_FIRST
#define XATTR_FILLCOLOR             (XATTR_FILL_FIRST + 1)      /* V3: 1019  V2: 1019 */
#define XATTR_FILLGRADIENT          (XATTR_FILL_FIRST + 2)      /* V3: 1020  V2: 1020 */
#define XATTR_FILLHATCH             (XATTR_FILL_FIRST + 3)      /* V3: 1021  V2: 1021 */
#define XATTR_FILLBITMAP            (XATTR_FILL_FIRST + 4)      /* V3: 1022  V2: 1022 */
#define XATTR_FILLTRANSPARENCE      (XATTR_FILL_FIRST + 5)      /* V3: 1023  V2: 1023 */
#define XATTR_GRADIENTSTEPCOUNT     (XATTR_FILL_FIRST + 6)      /* V3: 1024  V2: 1024 */
#define XATTR_FILLBMP_TILE          (XATTR_FILL_FIRST + 7)      /* V3: 1025  V2: 1025 */
#define XATTR_FILLBMP_POS           (XATTR_FILL_FIRST + 8)      /* V3: 1026  V2: 1026 */
#define XATTR_FILLBMP_SIZEX         (XATTR_FILL_FIRST + 9)      /* V3: 1027  V2: 1027 */
#define XATTR_FILLBMP_SIZEY         (XATTR_FILL_FIRST + 10)     /* V3: 1028  V2: 1028 */
#define XATTR_FILLFLOATTRANSPARENCE (XATTR_FILL_FIRST + 11)     /* V3: 1029  V2: 1029 */
#define XATTR_SECONDARYFILLCOLOR    (XATTR_FILL_FIRST + 12)     /* V3: 1030 */
#define XATTR_FILLBMP_SIZELOG       (XATTR_FILL_FIRST + 13)     /* V3: 1031 */
#define XATTR_FILLBMP_TILEOFFSETX   (XATTR_FILL_FIRST + 14)     /* V3: 1032 */
#define XATTR_FILLBMP_TILEOFFSETY   (XATTR_FILL_FIRST + 15)     /* V3: 1033 */
#define XATTR_FILLBMP_STRETCH       (XATTR_FILL_FIRST + 16)     /* V3: 1034 */
#define XATTR_FILLBMP_POSOFFSETX    (XATTR_FILL_FIRST + 17)     /* V3: 1041 */
#define XATTR_FILLBMP_POSOFFSETY    (XATTR_FILL_FIRST + 18)     /* V3: 1042 */
#define XATTR_FILLBACKGROUND        (XATTR_FILL_FIRST + 19)     /* V3: 1043 */
#define XATTR_FILL_LAST             XATTR_FILLBACKGROUND
#define XATTRSET_FILL           (XATTR_FILL_LAST + 1)           /* V3: 1047  V2: 1030 */

#define XATTR_TEXT_FIRST        (XATTRSET_FILL + 1)
#define XATTR_FORMTXTSTYLE      XATTR_TEXT_FIRST                /* V3: 1048  V2: 1031 */
#define XATTR_FORMTXTADJUST     (XATTR_TEXT_FIRST + 1)          /* V3: 1049  V2: 1032 */
#define XATTR_FORMTXTDISTANCE   (XATTR_TEXT_FIRST + 2)          /* V3: 1050  V2: 1033 */
#define XATTR_FORMTXTSTART      (XATTR_TEXT_FIRST + 3)          /* V3: 1051  V2: 1034 */
#define XATTR_FORMTXTMIRROR     (XATTR_TEXT_FIRST + 4)          /* V3: 1052  V2: 1035 */
#define XATTR_FORMTXTOUTLINE    (XATTR_TEXT_FIRST + 5)          /* V3: 1053  V2: 1036 */
#define XATTR_FORMTXTSHADOW     (XATTR_TEXT_FIRST + 6)          /* V3: 1054  V2: 1037 */
#define XATTR_FORMTXTSHDWCOLOR  (XATTR_TEXT_FIRST + 7)          /* V3: 1055  V2: 1038 */
#define XATTR_FORMTXTSHDWXVAL   (XATTR_TEXT_FIRST + 8)          /* V3: 1056  V2: 1039 */
#define XATTR_FORMTXTSHDWYVAL   (XATTR_TEXT_FIRST + 9)          /* V3: 1057  V2: 1040 */
#define XATTR_FORMTXTSTDFORM    (XATTR_TEXT_FIRST + 10)         /* V3: 1058  V2: 1041 */
#define XATTR_FORMTXTHIDEFORM   (XATTR_TEXT_FIRST + 11)         /* V3: 1059  V2: 1042 */
#define XATTR_FORMTXTSHDWTRANSP (XATTR_TEXT_FIRST + 12)         /* V3: 1060  V2: 1043 */
#define XATTR_TEXT_LAST         XATTR_FORMTXTSHDWTRANSP

#define XATTR_END               XATTR_TEXT_LAST

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
