/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Gert van Valkenhoef <g.h.m.van.valkenhoef@rug.nl>
 *  (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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

std::vector<TCHAR> OUStringToTCHARVec(rtl::OUString const &rStr);
rtl::OUString TCHARArrayToOUString(TCHAR const *str);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
