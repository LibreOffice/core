/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SB_SBDEF_HXX
#define _SB_SBDEF_HXX

#include <basic/sbxdef.hxx>
#include <rtl/ustring.hxx>
#include "basicdllapi.h"

// Returns type name for Basic type, array flag is ignored
// implementation: basic/source/runtime/methods.cxx
BASIC_DLLPUBLIC ::rtl::OUString getBasicTypeName( SbxDataType eType );

// Returns type name for Basic objects, especially
// important for SbUnoObj instances
// implementation: basic/source/classes/sbunoobj.cxx
class SbxObject;
BASIC_DLLPUBLIC ::rtl::OUString getBasicObjectTypeName( SbxObject* pObj );

// Allows Basic IDE to set watch mode to suppress errors
// implementation: basic/source/runtime/runtime.cxx
BASIC_DLLPUBLIC void setBasicWatchMode( bool bOn );

// Debug Flags:
#define SbDEBUG_BREAK       0x0001          // Break-Callback
#define SbDEBUG_STEPINTO    0x0002          // Single Step-Callback
#define SbDEBUG_STEPOVER    0x0004          // Additional flag Step Over
#define SbDEBUG_CONTINUE    0x0008          // Do not change flags
#define SbDEBUG_STEPOUT     0x0010          // Leave Sub

#define SBXID_BASIC         0x6273          // sb: StarBASIC
#define SBXID_BASICMOD      0x6d62          // bm: StarBASIC Module
#define SBXID_BASICPROP     0x7262          // pr: StarBASIC Property
#define SBXID_BASICMETHOD   0x6d65          // me: StarBASIC Method
#define SBXID_JSCRIPTMOD    0x6a62          // jm: JavaScript Module
#define SBXID_JSCRIPTMETH   0x6a64          // jm: JavaScript Module

#define SBX_HINT_BASICSTART     SFX_HINT_USER04
#define SBX_HINT_BASICSTOP      SFX_HINT_USER05

enum PropertyMode
{
    PROPERTY_MODE_NONE,
    PROPERTY_MODE_GET,
    PROPERTY_MODE_LET,
    PROPERTY_MODE_SET
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
