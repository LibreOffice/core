/*************************************************************************
 *
 *  $RCSfile: ThreadPoolManager.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-10-09 10:11:07 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
