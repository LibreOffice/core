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
/** The number of predefined format code indices that must be defined by
    locale data, except BOOLEAN and TEXT, exclusive.
    This later in the build is static asserted to be equal to
    NfIndexTableOffset::NF_INDEX_TABLE_RESERVED_START. */
constexpr sal_Int16 nStopPredefinedFormatIndex = 51;

/** The number of reserved (with defined meaning) built-in format code indices,
    additional locale data format codes can be defined starting at this index
    value.
    This later in the build is static asserted to be greater or equal to
    NfIndexTableOffset::NF_INDEX_TABLE_ENTRIES */
constexpr sal_Int16 nFirstFreeFormatIndex = 66;

/** This number shows month days without Jan and Feb.
 *  According to this, it is calcuated as (365-31-28)/10 = 30.6, but because
 *  of a floating point bug, it was used as 30.6001 as a workaround. It was a
 *  "workaround for the numerical errors in his hp-85 microcomputer from 1978".
 *
 *  "30.6001, 25 year old hack?"
 *  https://www.hpmuseum.org/cgi-sys/cgiwrap/hpmuseum/archv011.cgi?read=31650 */
constexpr double monthDaysWithoutJanFeb = (365 - 31 - 28) / 10.0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
