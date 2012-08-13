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
#ifndef _TOOLS_TENCCVT_HXX
#define _TOOLS_TENCCVT_HXX

#include <rtl/textenc.h>
#include "tools/toolsdllapi.h"

// Functions for handling Import/Export

/// return an encoding which has more defined Characters as the given
/// encoding, but have the same definition for the defined characters
/// e.g.: windows-1252 for iso-8859-1 or windows-1254 for iso-8859-9
TOOLS_DLLPUBLIC rtl_TextEncoding GetExtendedCompatibilityTextEncoding( rtl_TextEncoding eEncoding );

/// return an encoding which has more defined Characters as the given
/// encoding. The encodings could be different.
/// e.g.: windows-1251 for iso-8859-5
TOOLS_DLLPUBLIC rtl_TextEncoding GetExtendedTextEncoding( rtl_TextEncoding eEncoding );

/// if the given encoding is an multi-byte encoding (which allows more than
/// one byte per char, e.g. UTF-8 or Shift-JIS), a one-byte encoding
/// is returned (normally windows-1252).
TOOLS_DLLPUBLIC rtl_TextEncoding GetOneByteTextEncoding( rtl_TextEncoding eEncoding );

TOOLS_DLLPUBLIC rtl_TextEncoding GetSOLoadTextEncoding( rtl_TextEncoding eEncoding );
TOOLS_DLLPUBLIC rtl_TextEncoding GetSOStoreTextEncoding( rtl_TextEncoding eEncoding );

/**
 * Given a Unicode character, return a legacy Microsoft Encoding which
 * supports it. Returns RTL_TEXTENCODING_DONTKNOW if there is
 * no encoding which could support the character
 *
 * Useful as a utility to categorize unicode characters into the best fit
 * windows charset range for exporting to ww6 & wmf or as a hint to non \u
 * unicode token aware rtf readers
 */
TOOLS_DLLPUBLIC rtl_TextEncoding getBestMSEncodingByChar(sal_Unicode c);

#endif  // _TOOLS_TENCCVT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
