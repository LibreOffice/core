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

#ifndef INCLUDED_I18NUTIL_PAPER_HXX
#define INCLUDED_I18NUTIL_PAPER_HXX

#include <i18nutil/i18nutildllapi.h>
#include <rtl/string.hxx>
#include <com/sun/star/lang/Locale.hpp>

//!! The values of the following enumerators must correspond to the array position
//!! of the respective paper size in the file i18nutil/source/utility/paper.cxx
//!! Thus don't reorder the enum values here without changing the code there as well.
enum Paper
{
    PAPER_A0,
    PAPER_A1,
    PAPER_A2,
    PAPER_A3,
    PAPER_A4,
    PAPER_A5,
    PAPER_B4_ISO,
    PAPER_B5_ISO,
    PAPER_LETTER,
    PAPER_LEGAL,
    PAPER_TABLOID,
    PAPER_USER,
    PAPER_B6_ISO,
    PAPER_ENV_C4,
    PAPER_ENV_C5,
    PAPER_ENV_C6,
    PAPER_ENV_C65,
    PAPER_ENV_DL,
    PAPER_SLIDE_DIA,
    PAPER_SCREEN_4_3,
    PAPER_C,
    PAPER_D,
    PAPER_E,
    PAPER_EXECUTIVE,
    PAPER_FANFOLD_LEGAL_DE,
    PAPER_ENV_MONARCH,
    PAPER_ENV_PERSONAL,
    PAPER_ENV_9,
    PAPER_ENV_10,
    PAPER_ENV_11,
    PAPER_ENV_12,
    PAPER_KAI16,
    PAPER_KAI32,
    PAPER_KAI32BIG,
    PAPER_B4_JIS,
    PAPER_B5_JIS,
    PAPER_B6_JIS,
    PAPER_LEDGER,
    PAPER_STATEMENT,
    PAPER_QUARTO,
    PAPER_10x14,
    PAPER_ENV_14,
    PAPER_ENV_C3,
    PAPER_ENV_ITALY,
    PAPER_FANFOLD_US,
    PAPER_FANFOLD_DE,
    PAPER_POSTCARD_JP,
    PAPER_9x11,
    PAPER_10x11,
    PAPER_15x11,
    PAPER_ENV_INVITE,
    PAPER_A_PLUS,
    PAPER_B_PLUS,
    PAPER_LETTER_PLUS,
    PAPER_A4_PLUS,
    PAPER_DOUBLEPOSTCARD_JP,
    PAPER_A6,
    PAPER_12x11,
    PAPER_A7,
    PAPER_A8,
    PAPER_A9,
    PAPER_A10,
    PAPER_B0_ISO,
    PAPER_B1_ISO,
    PAPER_B2_ISO,
    PAPER_B3_ISO,
    PAPER_B7_ISO,
    PAPER_B8_ISO,
    PAPER_B9_ISO,
    PAPER_B10_ISO,
    PAPER_ENV_C2,
    PAPER_ENV_C7,
    PAPER_ENV_C8,
    PAPER_ARCHA,
    PAPER_ARCHB,
    PAPER_ARCHC,
    PAPER_ARCHD,
    PAPER_ARCHE,
    PAPER_SCREEN_16_9,
    PAPER_SCREEN_16_10,
    PAPER_16K_195x270,
    PAPER_16K_197x273
};

// defined for 'equal size' test with the implementation array
#define NUM_PAPER_ENTRIES   (PAPER_16K_197x273 - PAPER_A0 + 1)


class I18NUTIL_DLLPUBLIC PaperInfo
{
    Paper m_eType;
    long m_nPaperWidth;     // width in 100thMM
    long m_nPaperHeight;    // height in 100thMM
public:
    PaperInfo(Paper eType);
    PaperInfo(long nPaperWidth, long nPaperHeight);

    Paper getPaper() const { return m_eType; }
    long getWidth() const { return m_nPaperWidth; }
    long getHeight() const { return m_nPaperHeight; }
    bool sloppyEqual(const PaperInfo &rOther) const;
    void doSloppyFit();

    static PaperInfo getSystemDefaultPaper();
    static PaperInfo getDefaultPaperForLocale(const css::lang::Locale & rLocale);

    static Paper fromPSName(const OString &rName);
    static OString toPSName(Paper eType);

    static long sloppyFitPageDimension(long nDimension);
};

#endif // INCLUDED_I18NUTIL_PAPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
