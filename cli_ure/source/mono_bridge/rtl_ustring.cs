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

namespace uno.rtl {

using System;
using System.Runtime.InteropServices;
using System.Text;

public unsafe struct UString
{
    public int RefCount;
    public int Length;
    public char FirstChar;

    [DllImport("sal")]
    private static extern void rtl_uString_acquire(void* data);

    [DllImport("sal")]
    private static unsafe extern void rtl_uString_release(void* data);

    [DllImport("sal")]
    private static unsafe extern void rtl_uString_new(void* data);

    [DllImport("sal")]
    private static unsafe extern void rtl_uString_newFromStr_WithLength(
        void* data,
        // this should pass a pointer to the original string's char[]
        [MarshalAs(UnmanagedType.LPWStr)] string value,
        int len);

    [DllImport("sal")]
    private static unsafe extern void rtl_uString_newFromStr_WithLength(
        void* data,
        // this should pass a pointer to the stringbuilder's internal char[]
        [MarshalAs(UnmanagedType.LPWStr)] StringBuilder buffer,
        int len);

    public static unsafe void Acquire(UString* us)
    {
        rtl_uString_acquire(us);
    }

    public static unsafe void Release(UString* us)
    {
        rtl_uString_release(us);
    }

    public static unsafe void New(UString** p)
    {
        rtl_uString_new(p);
    }

    public static unsafe void NewFromString(UString **p, string s)
    {
        rtl_uString_newFromStr_WithLength(p, s, s.Length);
    }

    public static unsafe void NewFromStringBuilder(UString **p, StringBuilder sb)
    {
        rtl_uString_newFromStr_WithLength(p, sb, sb.Length);
    }

    public static unsafe string UStringToString(UString *p)
    {
        return new String(&(p->FirstChar), 0, p->Length);
    }
}

// FIXME move this to its own file or rename this file to e.g. sal
public unsafe struct Mem
{
    // FIXME parameter is a sal_Size which is unsigned and has the
    // size of a native long.  Thus this is not 64bit safe.  Might have
    // to write a glue function that always takes 32bit.
    [DllImport("sal", EntryPoint="rtl_allocateMemory")]
    public static unsafe extern void *Allocate(int bytes);
}

}
