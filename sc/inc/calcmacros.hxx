/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_INC_CALCMACROS_HXX
#define INCLUDED_SC_INC_CALCMACROS_HXX

#define DEBUG_COLUMN_STORAGE 0
#define DEBUG_PIVOT_TABLE 0
#define DEBUG_FORMULA_COMPILER 0
#define DEBUG_AREA_BROADCASTER 0

#define DUMP_COLUMN_STORAGE 0
#define DUMP_PIVOT_TABLE 0

#ifdef DBG_UTIL
#undef DUMP_COLUMN_STORAGE
#define DUMP_COLUMN_STORAGE 1
#undef DUMP_PIVOT_TABLE
#define DUMP_PIVOT_TABLE 1
#endif

#if DUMP_PIVOT_TABLE || DEBUG_PIVOT_TABLE || \
    DUMP_COLUMN_STORAGE || DEBUG_COLUMN_STORAGE || \
    DEBUG_FORMULA_COMPILER || \
    DEBUG_AREA_BROADCASTER
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
