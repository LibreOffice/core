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

#include <sal/types.h>

namespace i18npool {

/*
 * Thai character type definition.
 */

     0       // Control character
      1       // Non-composible
     2       // Consonant
       3       // Leading vowel
      4       // Following vowel
      5       // Following vowel
      6
      7       // Below vowel
      8
       9       // Below diacritic
     10      // Tone
      11      // Above diacritic
      12
      13
      14      // Above vowel
      15
      16

      17

const sal_uInt16 thaiCT[128] = {     // Thai character type
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

(x) ((x >= 0x0E00 && x < 0x0E60) ? thaiCT[x - 0x0E00] : CT_NON)

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
