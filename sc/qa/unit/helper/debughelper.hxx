/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_QA_UNIT_HELPER_DEBUGHELPER_HXX
#define INCLUDED_SC_QA_UNIT_HELPER_DEBUGHELPER_HXX

#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>

#ifdef WNT
#if !defined NOMINMAX
#define NOMINMAX
#endif
#include <prewin.h>
#include <postwin.h>
#undef NOMINMAX
#undef RGB
#endif

#include <iostream>
#include <vector>

#include <svl/gridprinter.hxx>

#define CALC_DEBUG_OUTPUT 0
#define CALC_TEST_PERF 0

using namespace ::com::sun::star;
using ::std::cout;
using ::std::cerr;
using ::std::endl;
using ::std::vector;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
