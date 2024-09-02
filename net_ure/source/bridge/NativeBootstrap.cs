/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Linq;

namespace com.sun.star.uno
{
    public static class NativeBootstrap
    {
        public static XComponentContext bootstrap()
        {
            Interop.bootstrap(null);
            return null;
        }

        private static class Interop
        {
            private const string BOOTSTRAP_LIBRARY = "net_bootstrap";

            [DllImport(BOOTSTRAP_LIBRARY, CallingConvention = CallingConvention.Cdecl)]
            public static extern IntPtr bootstrap([MarshalAs(UnmanagedType.LPWStr)] string sParams);
        }
    }
}
