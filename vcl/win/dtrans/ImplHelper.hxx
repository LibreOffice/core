/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include <sal/types.h>
#include <rtl/ustring.hxx>

#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <objidl.h>

// target device and formatetc helper
void DeleteTargetDevice(DVTARGETDEVICE* ptd);
bool CopyFormatEtc(LPFORMATETC petcDest, LPFORMATETC petcSrc);
sal_Int32 CompareFormatEtc( const FORMATETC* pFetcLeft, const FORMATETC* pFetcRight);
bool CompareTargetDevice(DVTARGETDEVICE* ptdLeft, DVTARGETDEVICE const * ptdRight);
DVTARGETDEVICE* CopyTargetDevice(DVTARGETDEVICE* ptdSrc);

// some codepage helper functions

// returns a windows codepage appropriate to the
// given mime charset parameter value

sal_uInt32 getWinCPFromMimeCharset(
    const OUString& charset );

// returns a windows codepage appropriate to the
// given locale and locale type

OUString getWinCPFromLocaleId(
    LCID lcid, LCTYPE lctype );

// returns a mime charset parameter value appropriate
// to the given codepage, optional a prefix can be
// given, e.g. "windows-" or "cp"

OUString getMimeCharsetFromWinCP(
    sal_uInt32 cp, const OUString& aPrefix );

// returns a mime charset parameter value appropriate
// to the given locale id and locale type, optional a
// prefix can be given, e.g. "windows-" or "cp"

OUString getMimeCharsetFromLocaleId(
    LCID lcid, LCTYPE lctype, const OUString& aPrefix  );

// returns true, if a given codepage is an oem codepage

bool IsOEMCP( sal_uInt32 codepage );

// converts a codepage into a string representation

OUString cptostr( sal_uInt32 codepage );

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
