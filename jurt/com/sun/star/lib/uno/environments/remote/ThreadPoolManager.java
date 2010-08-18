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

package com.sun.star.lib.uno.environments.remote;

/**
 * Manages the UNO thread pool factory.
 *
 * <P>The thread pool factory is a process-wide resource.  It is important that
 * all UNO environments within a process share the same thread pool mechanisms:
 * if a synchronous UNO call is bridged out from one local UNO environment over
 * one remote bridge, and recursively calls back into another local UNO
 * environment over another remote bridge, the code in the second environment
 * should be executed in the thread that did the original call from the first
 * environment.</P>
 *
 * <P>There are both a Java and a native thread pool factory.  A pure Java
 * process will always use the Java thread pool factory.  A mixed process uses
 * the system property <CODE>org.openoffice.native</CODE> (to be set by the
 * native code that starts the JVM) to determine which implementation
 * to use.</P>
 */
public final class ThreadPoolManager {
    /**
     * Creates a thread pool instance.
     *
     * @return a new thread pool instance; will never be <CODE>null</CODE>
     */
    public static synchronized IThreadPool create() {
        if (useNative) {
            return new NativeThreadPool();
        } else {
            if (javaFactory == null) {
                javaFactory = new JavaThreadPoolFactory();
            }
            return javaFactory.createThreadPool();
        }
    }

    /**
     * Leads to using the native thread pool factory, unless a Java thread pool
     * has already been created.
     *
     * @return <CODE>false</CODE> if a Java thread pool has already been created
     */
    public static synchronized boolean useNative() {
        useNative = javaFactory == null;
        return useNative;
    }

    private static boolean useNative
    = System.getProperty("org.openoffice.native") != null;
    private static JavaThreadPoolFactory javaFactory = null;

    private ThreadPoolManager() {} // do not instantiate
}
