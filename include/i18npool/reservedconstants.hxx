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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
