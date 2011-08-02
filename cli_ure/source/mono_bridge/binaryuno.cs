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

namespace uno.Binary {

using System;
using System.Runtime.InteropServices;
using uno.Typelib;
using uno.rtl;

[ StructLayout(LayoutKind.Sequential) ]
public unsafe struct Any
{
    /** type of value
     */
    public uno.Typelib.TypeDescriptionReference *pType;

    /** pointer to value; this may point to pReserved and thus the uno_Any is not anytime
        mem-copyable! You may have to correct the pData pointer to pReserved. Otherwise you need
        not, because the data is stored in heap space.
    */
    public void *pData;
    /** reserved space for storing value
    */
    public void *pReserved;

    [ DllImport("uno_cppu", EntryPoint="uno_any_construct") ]
    public static unsafe extern void Construct(/* Any */ void *dest,
                           void *source,
                           /* uno.Typelib.TypeDescriptionReference */ void *type,
                           void *acquireFunction);

    [ DllImport("uno_cppu", EntryPoint="uno_any_destruct") ]
    public static unsafe extern void Destroy(/* Any */ void *value, void *releaseFunction);
}

// FIXME wrap this nicely
public struct Interface
{
    [ DllImport("cli_uno", EntryPoint="cli_uno_interface_acquire") ]
    public static extern void Acquire(IntPtr unoInterface);

    [ DllImport("cli_uno", EntryPoint="cli_uno_interface_release") ]
    public static extern void Release(IntPtr unoInterface);

    [ DllImport("cli_uno", EntryPoint="cli_uno_interface_dispatch") ]
    public static unsafe extern void Dispatch(IntPtr unoInterface,
                                              /* uno.Typelib.TypeDescription */ void *memberTD,
                                              void *result,
                                              void **args,
                                              uno.Binary.Any **exception);
}

// FIXME and this
public class Environment
{
    [ DllImport("cli_uno", EntryPoint="cli_uno_environment_getObjectIdentifier") ]
    public static unsafe extern void GetObjectIdentifier(IntPtr unoEnvironment,
                             UString** oid,
                             IntPtr unoInterface);

    [ DllImport("cli_uno", EntryPoint="cli_uno_environment_registerInterface") ]
    public static unsafe extern void RegisterInterface(
        IntPtr unoEnvironment,
        ref IntPtr ppInterface,
        /* UString */ void* oid,
        /* InterfaceTypeDescription */ void *td);

    [ DllImport("cli_uno", EntryPoint="cli_uno_environment_getRegisteredInterface") ]
    public static unsafe extern void GetRegisteredInterface(
        IntPtr unoEnvironment,
        ref IntPtr ppInterface,
        /* UString */ void* oid,
        /* InterfaceTypeDescription */ void *td);
}

public struct Data
{
    [ DllImport("uno_cppu", EntryPoint="uno_type_destructData") ]
    public static unsafe extern void Destroy(void *data,
                                             /* uno.Typelib.TypeDescription */ void *td,
                                             // FIXME is this okay? release is a function pointer
                                             void *release);
}

public unsafe struct SequencePtr
{
    int *sal_Sequence;

    // 	sal_Int32			nRefCount;
    // 	sal_Int32			nElements;
    // 	char				elements[1];

    /** element count<br>
    */
    /** elements array<br>
    */

    /** reference count of sequence<br>
    */
    private unsafe int nRefCount
    {
        get { return *(sal_Sequence); }
        set { *(sal_Sequence) = value; }
    }

    public unsafe int nElements
    {
        get { return *(sal_Sequence + 1); }
        set { *(sal_Sequence + 1) = value; }
    }

    public unsafe IntPtr elementsPtr
    {
        get { return new IntPtr((void *)(sal_Sequence + 2)); }
    }

    private unsafe SequencePtr(void *mem)
    {
        sal_Sequence = (int*)mem;
    }

    private const int HEADER_SIZE = 8; // FIXME alignment

    public static SequencePtr Allocate(int length, int elementSize)
    {
        void *rtlPtr = uno.rtl.Mem.Allocate(HEADER_SIZE + (length * elementSize));
        SequencePtr seqPtr = new SequencePtr(rtlPtr);
        seqPtr.nRefCount = 1;
        seqPtr.nElements = length;
        return seqPtr;
    }
}
}
