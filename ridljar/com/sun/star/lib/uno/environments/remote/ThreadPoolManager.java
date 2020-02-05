/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
     * @return a new thread pool instance; will never be <CODE>null</CODE>.
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
     * @return <CODE>false</CODE> if a Java thread pool has already been created.
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
