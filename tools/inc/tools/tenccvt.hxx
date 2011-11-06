/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _TOOLS_TENCCVT_HXX
#define _TOOLS_TENCCVT_HXX

#include <rtl/textenc.h>
#ifndef _TOOLS_SOLAR_H
#include <tools/solar.h>
#endif
#include "tools/toolsdllapi.h"

// ----------------------------------------
// - Functions for handling Import/Export -
// ----------------------------------------

// return an encoding which has more defined Characters as the given
// encoding, but have the same definition for the defined characters
// e.g.: windows-1252 for iso-8859-1 or windows-1254 for iso-8859-9
TOOLS_DLLPUBLIC rtl_TextEncoding GetExtendedCompatibilityTextEncoding( rtl_TextEncoding eEncoding );

// return an encoding which has more defined Characters as the given
// encoding. The encodings could be different.
// e.g.: windows-1251 for iso-8859-5
TOOLS_DLLPUBLIC rtl_TextEncoding GetExtendedTextEncoding( rtl_TextEncoding eEncoding );

// if the given encoding is an multi-byte encoding (which allows more than
// one byte per char, e.g. UTF-8 or Shift-JIS), a one-byte encoding
// is returned (normally windows-1252).
TOOLS_DLLPUBLIC rtl_TextEncoding GetOneByteTextEncoding( rtl_TextEncoding eEncoding );

TOOLS_DLLPUBLIC rtl_TextEncoding GetSOLoadTextEncoding( rtl_TextEncoding eEncoding, sal_uInt16 nVersion = SOFFICE_FILEFORMAT_50 );
TOOLS_DLLPUBLIC rtl_TextEncoding GetSOStoreTextEncoding( rtl_TextEncoding eEncoding, sal_uInt16 nVersion = SOFFICE_FILEFORMAT_50 );

#endif  // _TOOLS_TENCCVT_HXX
