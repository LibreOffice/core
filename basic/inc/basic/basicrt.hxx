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
#ifndef _BASICRT_HXX
#define _BASICRT_HXX

#include <tools/string.hxx>
#include <basic/sbxdef.hxx>

class SbiRuntime;
class SbErrorStackEntry;

class BasicRuntime
{
    SbiRuntime* pRun;
public:
    BasicRuntime( SbiRuntime* p ) : pRun ( p ){;}
    const String GetSourceRevision();
    const String GetModuleName( SbxNameType nType );
    const String GetMethodName( SbxNameType nType );
    xub_StrLen GetLine();
    xub_StrLen GetCol1();
    xub_StrLen GetCol2();
    sal_Bool IsRun();
    sal_Bool IsValid() { return pRun != NULL; }
    BasicRuntime GetNextRuntime();
};

class BasicErrorStackEntry
{
    SbErrorStackEntry *pEntry;
public:
    BasicErrorStackEntry( SbErrorStackEntry *p ) : pEntry ( p ){;}
    const String GetSourceRevision();
    const String GetModuleName( SbxNameType nType );
    const String GetMethodName( SbxNameType nType );
    xub_StrLen GetLine();
    xub_StrLen GetCol1();
    xub_StrLen GetCol2();
};

class BasicRuntimeAccess
{
public:
    static BasicRuntime GetRuntime();
    static bool HasRuntime();
    static sal_uInt16 GetStackEntryCount();
    static BasicErrorStackEntry GetStackEntry( sal_uInt16 nIndex );
    static sal_Bool HasStack();
    static void DeleteStack();

    static sal_Bool IsRunInit();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
