/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

using System;
using System.Runtime.InteropServices;

namespace com.sun.star.uno.native
{
    [StructLayout(LayoutKind.Explicit, CharSet = CharSet.Unicode)]
    internal struct InteropValue
    {
        [FieldOffset(0)] public byte boolData; // (System.Bool is not blittable)
        [FieldOffset(0)] public sbyte byteData;
        [FieldOffset(0)] public char charData;
        [FieldOffset(0)] public short shortData;
        [FieldOffset(0)] public ushort unsigShortData;
        [FieldOffset(0)] public int longData;
        [FieldOffset(0)] public uint unsigLongData;
        [FieldOffset(0)] public long hyperData;
        [FieldOffset(0)] public ulong unsigHyperData;
        [FieldOffset(0)] public float floatData;
        [FieldOffset(0)] public double doubleData;
        [FieldOffset(0)] public IntPtr stringData;
        [FieldOffset(0)] public IntPtr typeData;
        [FieldOffset(0)] public Any anyData;
        [FieldOffset(0)] public int enumData;
        [FieldOffset(0)] public IntPtr structData;
        [FieldOffset(0)] public IntPtr exceptionData;
        [FieldOffset(0)] public Sequence sequenceData;
        [FieldOffset(0)] public IntPtr interfaceData;

        [StructLayout(LayoutKind.Sequential)]
        public struct Any
        {
            public IntPtr data;
            public IntPtr type;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct Sequence
        {
            public IntPtr data;
            public int length;
        }
    };

    [UnmanagedFunctionPointer(CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    internal delegate IntPtr CreateProxyFunc(string pOid, string pInterfaceName, IntPtr pBridge,
        IntPtr pUnoInterface, IntPtr pTypeDesc);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    internal delegate string LookupObjectIdFunc(IntPtr pNetInterface);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    internal delegate IntPtr RegisterInterfaceFunc(IntPtr pNetInterface, string pOid, string pInterfaceName);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    internal delegate IntPtr LookupInterfaceFunc(string pOid, string pInterfaceName);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    internal delegate void RevokeInterfaceFunc(string pOid, string pInterfaceName);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    internal delegate byte DispatchCallFunc(IntPtr pNetInterface, string pMethodName,
        IntPtr pArgs, IntPtr pRet, IntPtr pExc);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    internal delegate void ThrowErrorFunc(string pMessage, string pWhere);

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    internal struct InteropContext
    {
        public CreateProxyFunc createProxy;
        public LookupObjectIdFunc lookupObjectId;
        public RegisterInterfaceFunc registerInterface;
        public LookupInterfaceFunc lookupInterface;
        public RevokeInterfaceFunc revokeInterface;
        public DispatchCallFunc dispatchCall;
        public ThrowErrorFunc throwError;
    };
}
