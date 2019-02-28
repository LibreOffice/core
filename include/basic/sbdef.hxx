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

#ifndef INCLUDED_BASIC_SBDEF_HXX
#define INCLUDED_BASIC_SBDEF_HXX

#include <basic/sbxdef.hxx>
#include <rtl/ustring.hxx>
#include <basic/basicdllapi.h>
#include <o3tl/typed_flags_set.hxx>

class ErrCode;

// Returns type name for Basic type, array flag is ignored
// implementation: basic/source/runtime/methods.cxx
BASIC_DLLPUBLIC OUString getBasicTypeName( SbxDataType eType );

// Returns type name for Basic objects, especially
// important for SbUnoObj instances
// implementation: basic/source/classes/sbunoobj.cxx
class SbxObject;
BASIC_DLLPUBLIC OUString getBasicObjectTypeName( SbxObject* pObj );

// Allows Basic IDE to set watch mode to suppress errors
// implementation: basic/source/runtime/runtime.cxx
BASIC_DLLPUBLIC void setBasicWatchMode( bool bOn );

// Debug Flags:
enum class BasicDebugFlags {
    NONE        = 0x0000,
    Break       = 0x0001,        // Break-Callback
    StepInto    = 0x0002,        // Single Step-Callback
    StepOver    = 0x0004,        // Additional flag Step Over
    Continue    = 0x0008,        // Do not change flags
    StepOut     = 0x0010,        // Leave Sub
};
namespace o3tl {
    template<> struct typed_flags<BasicDebugFlags> : is_typed_flags<BasicDebugFlags, 0x001f> {};
}

#define SBXID_BASIC         0x6273          // sb: StarBASIC
#define SBXID_BASICMOD      0x6d62          // bm: StarBASIC Module
#define SBXID_BASICPROP     0x7262          // pr: StarBASIC Property
#define SBXID_BASICMETHOD   0x6d65          // me: StarBASIC Method
#define SBXID_JSCRIPTMOD    0x6a62          // jm: JavaScript Module
#define SBXID_JSCRIPTMETH   0x6a64          // jm: JavaScript Module

enum class PropertyMode
{
    NONE,
    Get,
    Let,
    Set
};

BASIC_DLLPUBLIC extern std::pair<const char*, ErrCode> const RID_BASIC_START[];
BASIC_DLLPUBLIC std::locale BasResLocale();
BASIC_DLLPUBLIC OUString BasResId(const char* pId);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
