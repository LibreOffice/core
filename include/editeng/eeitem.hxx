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

#ifndef INCLUDED_EDITENG_EEITEM_HXX
#define INCLUDED_EDITENG_EEITEM_HXX

#include <svl/solar.hrc>

#define EE_ITEMS_START              (OWN_ATTR_VALUE_END+1)

// Paragraph attributes:
#define EE_PARA_START               (EE_ITEMS_START+0)
#define EE_PARA_WRITINGDIR          (EE_ITEMS_START+0)
#define EE_PARA_XMLATTRIBS          (EE_ITEMS_START+1)
#define EE_PARA_HANGINGPUNCTUATION  (EE_ITEMS_START+2)
#define EE_PARA_FORBIDDENRULES      (EE_ITEMS_START+3)
#define EE_PARA_ASIANCJKSPACING     (EE_ITEMS_START+4)
#define EE_PARA_NUMBULLET           (EE_ITEMS_START+5)
#define EE_PARA_HYPHENATE           (EE_ITEMS_START+6)
#define EE_PARA_BULLETSTATE         (EE_ITEMS_START+7)
#define EE_PARA_OUTLLRSPACE         (EE_ITEMS_START+8)
#define EE_PARA_OUTLLEVEL           (EE_ITEMS_START+9)
#define EE_PARA_BULLET              (EE_ITEMS_START+10)
#define EE_PARA_LRSPACE             (EE_ITEMS_START+11)
#define EE_PARA_ULSPACE             (EE_ITEMS_START+12)
#define EE_PARA_SBL                 (EE_ITEMS_START+13)
#define EE_PARA_JUST                (EE_ITEMS_START+14)
#define EE_PARA_TABS                (EE_ITEMS_START+15)
#define EE_PARA_JUST_METHOD         (EE_ITEMS_START+16)
#define EE_PARA_VER_JUST            (EE_ITEMS_START+17)
#define EE_PARA_END                 (EE_ITEMS_START+17)

// Character attributes:
#define EE_CHAR_START               (EE_ITEMS_START+18)
#define EE_CHAR_COLOR               (EE_ITEMS_START+18)
#define EE_CHAR_FONTINFO            (EE_ITEMS_START+19)
#define EE_CHAR_FONTHEIGHT          (EE_ITEMS_START+20)
#define EE_CHAR_FONTWIDTH           (EE_ITEMS_START+21)
#define EE_CHAR_WEIGHT              (EE_ITEMS_START+22)
#define EE_CHAR_UNDERLINE           (EE_ITEMS_START+23)
#define EE_CHAR_STRIKEOUT           (EE_ITEMS_START+24)
#define EE_CHAR_ITALIC              (EE_ITEMS_START+25)
#define EE_CHAR_OUTLINE             (EE_ITEMS_START+26)
#define EE_CHAR_SHADOW              (EE_ITEMS_START+27)
#define EE_CHAR_ESCAPEMENT          (EE_ITEMS_START+28)
#define EE_CHAR_PAIRKERNING         (EE_ITEMS_START+29)
#define EE_CHAR_KERNING             (EE_ITEMS_START+30)
#define EE_CHAR_WLM                 (EE_ITEMS_START+31)
#define EE_CHAR_LANGUAGE            (EE_ITEMS_START+32)
#define EE_CHAR_LANGUAGE_CJK        (EE_ITEMS_START+33)
#define EE_CHAR_LANGUAGE_CTL        (EE_ITEMS_START+34)
#define EE_CHAR_FONTINFO_CJK        (EE_ITEMS_START+35)
#define EE_CHAR_FONTINFO_CTL        (EE_ITEMS_START+36)
#define EE_CHAR_FONTHEIGHT_CJK      (EE_ITEMS_START+37)
#define EE_CHAR_FONTHEIGHT_CTL      (EE_ITEMS_START+38)
#define EE_CHAR_WEIGHT_CJK          (EE_ITEMS_START+39)
#define EE_CHAR_WEIGHT_CTL          (EE_ITEMS_START+40)
#define EE_CHAR_ITALIC_CJK          (EE_ITEMS_START+41)
#define EE_CHAR_ITALIC_CTL          (EE_ITEMS_START+42)
#define EE_CHAR_EMPHASISMARK        (EE_ITEMS_START+43)
#define EE_CHAR_RELIEF              (EE_ITEMS_START+44)
#define EE_CHAR_RUBI_DUMMY          (EE_ITEMS_START+45)
#define EE_CHAR_XMLATTRIBS          (EE_ITEMS_START+46)
#define EE_CHAR_OVERLINE            (EE_ITEMS_START+47)
#define EE_CHAR_CASEMAP             (EE_ITEMS_START+48)
#define EE_CHAR_GRABBAG             (EE_ITEMS_START+49)
#define EE_CHAR_END                 (EE_ITEMS_START+49)

#define EE_FEATURE_START            (EE_ITEMS_START+50)
#define EE_FEATURE_TAB              (EE_ITEMS_START+50)
#define EE_FEATURE_LINEBR           (EE_ITEMS_START+51)
#define EE_FEATURE_NOTCONV          (EE_ITEMS_START+52)
#define EE_FEATURE_FIELD            (EE_ITEMS_START+53)
#define EE_FEATURE_END              (EE_ITEMS_START+53)

#define EE_ITEMS_END                (EE_ITEMS_START+53)

#define EDITITEMCOUNT ( EE_ITEMS_END - EE_ITEMS_START + 1 )

#endif // _ INCLUDED_EDITENG_EEITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
