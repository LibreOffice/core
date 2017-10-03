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
#ifndef INCLUDED_I18NPOOL_INC_WTT_H
#define INCLUDED_I18NPOOL_INC_WTT_H

namespace i18npool {

/*
 * Thai character type definition.
 */

#define CT_CTRL     0       // Control character
#define CT_NON      1       // Non-composible
#define CT_CONS     2       // Consonant
#define CT_LV       3       // Leading vowel
#define CT_FV1      4       // Following vowel
#define CT_FV2      5       // Following vowel
#define CT_FV3      6
#define CT_BV1      7       // Below vowel
#define CT_BV2      8
#define CT_BD       9       // Below diacritic
#define CT_TONE     10      // Tone
#define CT_AD1      11      // Above diacritic
#define CT_AD2      12
#define CT_AD3      13
#define CT_AV1      14      // Above vowel
#define CT_AV2      15
#define CT_AV3      16

#define MAX_CT      17

static const sal_uInt16 thaiCT[128] = {     // Thai character type
    CT_NON, CT_CONS, CT_CONS, CT_CONS, CT_CONS,CT_CONS, CT_CONS, CT_CONS,       //0E00
    CT_CONS, CT_CONS, CT_CONS, CT_CONS, CT_CONS, CT_CONS, CT_CONS, CT_CONS,
    CT_CONS, CT_CONS, CT_CONS, CT_CONS, CT_CONS, CT_CONS, CT_CONS, CT_CONS,     //0E10
    CT_CONS, CT_CONS, CT_CONS, CT_CONS, CT_CONS, CT_CONS, CT_CONS, CT_CONS,
    CT_CONS, CT_CONS, CT_CONS, CT_CONS, CT_FV3, CT_CONS, CT_FV3, CT_CONS,       //0E20
    CT_CONS, CT_CONS, CT_CONS, CT_CONS, CT_CONS, CT_CONS, CT_CONS, CT_NON,
    CT_FV1, CT_AV2, CT_FV1, CT_FV1, CT_AV1, CT_AV3, CT_AV2, CT_AV3,         //0E30
    CT_BV1, CT_BV2, CT_BD, CT_NON, CT_NON, CT_NON, CT_NON, CT_NON,
    CT_LV, CT_LV, CT_LV, CT_LV, CT_LV, CT_FV2, CT_NON, CT_AD2,          //0E40
    CT_TONE, CT_TONE, CT_TONE, CT_TONE, CT_AD1, CT_AD1, CT_AD3, CT_NON,
    CT_NON, CT_NON, CT_NON, CT_NON, CT_NON, CT_NON, CT_NON, CT_NON,         //0E50
    CT_NON, CT_NON, CT_NON, CT_NON, CT_NON, CT_NON, CT_NON, CT_CTRL };

#define getCharType(x) ((x >= 0x0E00 && x < 0x0E60) ? thaiCT[x - 0x0E00] : CT_NON)

}

#endif // INCLUDED_I18NPOOL_INC_WTT_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
