// -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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

package com.sun.star.bridges.jni_uno;

import com.sun.star.lib.util.NativeLibraryLoader;

//==============================================================================
public final class JNI_info_holder
{
    static {
        if (System.getProperty("java.vendor") == "The Android Project") {
            // See corresponding code in
            // javaunohelper/com/sun/star/comp/helper/Bootstrap.java for more
            // comments.

            boolean disable_dynloading = false;
            try {
                System.loadLibrary("lo-bootstrap");
            } catch (UnsatisfiedLinkError e) {
                disable_dynloading = true;
            }

            if (!disable_dynloading)
                NativeLibraryLoader.loadLibrary(JNI_info_holder.class.getClassLoader(),
                                                "java_uno");
    } else
        NativeLibraryLoader.loadLibrary(JNI_info_holder.class.getClassLoader(),
                                        "java_uno");
    }

    private static JNI_info_holder s_holder = new JNI_info_holder();

    private static long s_jni_info_handle;

    //__________________________________________________________________________
    private native void finalize( long jni_info_handle );

    //__________________________________________________________________________
    protected void finalize()
    {
        finalize( s_jni_info_handle );
    }
}

// vim:set shiftwidth=4 softtabstop=4 expandtab:
