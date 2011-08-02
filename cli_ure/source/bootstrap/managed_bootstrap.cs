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

namespace uno.util
{

using System;
using System.Collections;
using System.Runtime.InteropServices;

public class Bootstrap
{
    private Bootstrap() {}

    public static unoidl.com.sun.star.uno.XComponentContext
        defaultBootstrap_InitialComponentContext()
    {
        return defaultBootstrap_InitialComponentContext(null, null);
    }

    public static unoidl.com.sun.star.uno.XComponentContext
        defaultBootstrap_InitialComponentContext(
            string iniFile,
            IDictionaryEnumerator bootstrapParameters)
    {
        if (bootstrapParameters != null)
        {
            bootstrapParameters.Reset();
            while (bootstrapParameters.MoveNext())
            {
                string key = (string)bootstrapParameters.Key;
                string value = (string)bootstrapParameters.Value;

                native_bootstrap_set(key, key.Length, value, value.Length);
            }
        }

    System.Console.WriteLine("Bootstrap with ini " + iniFile);
        // bootstrap native uno
        IntPtr context;
        if (iniFile == null)
        {
            context = native_defaultBootstrap_InitialComponentContext();
        }
        else
        {
            context = native_defaultBootstrap_InitialComponentContext(iniFile, iniFile.Length);
        }

        return (unoidl.com.sun.star.uno.XComponentContext)ExtractObject(context);
    }

    public static unoidl.com.sun.star.uno.XComponentContext bootstrap()
    {
        return (unoidl.com.sun.star.uno.XComponentContext)ExtractObject(native_bootstrap());
    }

    static object ExtractObject(IntPtr managed)
    {
        GCHandle handle = (GCHandle)managed;
        object ret = handle.Target;
        handle.Free();
        return ret;
    }

    [DllImport("cli_uno_glue")]
    private static extern void native_bootstrap_set(
        [MarshalAs(UnmanagedType.LPWStr)] string key, int keyLength,
        [MarshalAs(UnmanagedType.LPWStr)] string value, int valueLength);

    [DllImport("cli_uno_glue", EntryPoint="native_defaultBootstrap_InitialComponentContext")]
    private static extern IntPtr native_defaultBootstrap_InitialComponentContext();

    [DllImport("cli_uno_glue", EntryPoint="native_defaultBootstrap_InitialComponentContext_iniFile")]
    private static extern IntPtr native_defaultBootstrap_InitialComponentContext(
        [MarshalAs(UnmanagedType.LPWStr)] string iniFile, int nameLength);

    [DllImport("cli_uno_glue")]
    private static extern IntPtr native_bootstrap();
}

}
