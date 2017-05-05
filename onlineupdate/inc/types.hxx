/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ONLINEUPDATE_TYPES_HXX
#define INCLUDED_ONLINEUPDATE_TYPES_HXX

#include <string>

#if defined(_WIN32)
typedef std::wstring tstring;
typedef WCHAR NS_tchar;
#else
typedef std::string tstring;
typedef char NS_tchar;
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
