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
    internal static class InteropMethods
    {
        private const string BOOTSTRAP_LIBRARY = "net_bootstrap";
        private const string UNO_LIBRARY = "net_uno";

        [DllImport(BOOTSTRAP_LIBRARY, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern IntPtr bootstrap(InteropContext pCtx);

        [DllImport(BOOTSTRAP_LIBRARY, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern IntPtr defaultBootstrap_InitialComponentContext(
            [MarshalAs(UnmanagedType.LPWStr)] string sIniFile,
            [MarshalAs(UnmanagedType.LPWStr)] string sParams,
            InteropContext pCtx
        );

        [DllImport(UNO_LIBRARY, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern IntPtr allocateMemory(int nBytes);

        [DllImport(UNO_LIBRARY, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void freeMemory(IntPtr pMemory);

        [DllImport(UNO_LIBRARY, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void releaseProxy(IntPtr pBridgeHandle,
            IntPtr pInterfaceHandle, IntPtr pTypeHandle);

        [DllImport(UNO_LIBRARY, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static unsafe extern byte dispatchCall(
            IntPtr pBridgeHandle,
            IntPtr pInterfaceHandle,
            IntPtr pTypeHandle,
            [MarshalAs(UnmanagedType.LPWStr)] string sMethod,
            InteropValue* pNetArgs,
            InteropValue* pNetRet,
            InteropValue* pNetExc
        );
    }
}
