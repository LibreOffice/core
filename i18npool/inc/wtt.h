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
#ifndef _I18N_WTT_H_
#define _I18N_WTT_H_

namespace com { namespace sun { namespace star { namespace i18n {

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

} } } }

#endif // _I18N_WTT_H_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
