/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_CALCMACROS_HXX
#define SC_CALCMACROS_HXX

#define DEBUG_COLUMN_STORAGE 0
#define DEBUG_PIVOT_TABLE 0
#define DEBUG_FORMULA_COMPILER 1

#if DEBUG_PIVOT_TABLE || DEBUG_COLUMN_STORAGE || DEBUG_FORMULA_COMPILER
#include <iostream>
#include <string>
#include <cstdio>
using std::cout;
using std::cerr;
using std::endl;
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
