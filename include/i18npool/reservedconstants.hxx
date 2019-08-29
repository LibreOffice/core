/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>

/** Constant values shared between i18npool and, for example, the number formatter. */
namespace i18npool
{
/** The number of reserved (with defined meaning) built-in format code indices,
    additional locale data format codes can be defined starting at this index
    value. */
constexpr sal_Int16 nFirstFreeFormatIndex = 60;

// tdf#51611 - Moved from sw/inc/hintids.hxx - Special characters added by hints.
#define CH_TXTATR_BREAKWORD u'\x0001'
#define CH_TXTATR_INWORD u'\xFFF9'

#define CH_TXT_ATR_INPUTFIELDSTART u'\x0004'
#define CH_TXT_ATR_INPUTFIELDEND u'\x0005'
#define CH_TXT_ATR_FORMELEMENT u'\x0006'

#define CH_TXT_ATR_FIELDSTART u'\x0007'
#define CH_TXT_ATR_FIELDEND u'\x0008'

/**
* Zero width space - Unicode 4.0.1 changed U+200B ZERO WIDTH SPACE from a Space Separator (Zs)
* to a Format Control(Cf). Since then, isWhitespace(0x200b) returns false.
* See http://www.unicode.org/versions/Unicode4.0.1/
*/
#define CH_ZERO_WIDTH_SPACE u'\x200B'
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
