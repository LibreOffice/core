/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_SOURCE_CORE_INC_FETAB_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_FETAB_HXX

#include <sal/config.h>

class SwFrame;
class SwTabFrame;
class SwTable;

extern SwFrame const * g_pColumnCacheLastCellFrame;
extern SwTabFrame const * g_pColumnCacheLastTabFrame;
extern SwTable const * g_pColumnCacheLastTable;
extern SwFrame const * g_pRowCacheLastCellFrame;
extern SwTabFrame const * g_pRowCacheLastTabFrame;
extern SwTable const * g_pRowCacheLastTable;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
