/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef LUCENEHELPER_HXX
#define LUCENEHELPER_HXX

#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4068 4263 4264 4266)
#endif

#if defined(__GNUC__) && defined(HAVE_GCC_VISIBILITY_FEATURE)
#  pragma GCC visibility push (default)
#endif

#include <CLucene.h>
#include <CLucene/analysis/LanguageBasedAnalyzer.h>

#if defined(__GNUC__) && defined(HAVE_GCC_VISIBILITY_FEATURE)
#  pragma GCC visibility pop
#endif

#if defined _MSC_VER
#pragma warning(pop)
#endif

#include <rtl/ustring.hxx>
#include <vector>

std::vector<TCHAR> OUStringToTCHARVec(OUString const &rStr);
OUString TCHARArrayToOUString(TCHAR const *str);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
