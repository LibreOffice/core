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

import complexlib.ComplexTestCase;

public final class JavaThreadPoolFactory_Test extends ComplexTestCase {
    public String[] getTestMethodNames() {
        return new String[] { "test" };
    }

    public void test() throws InterruptedException {
        ThreadId i1 = JavaThreadPoolFactory.getThreadId();
        assure(i1.equals(JavaThreadPoolFactory.getThreadId()));
        final ThreadId[] i2 = new ThreadId[1];
        new Thread() {
            public void run() {
                synchronized (i2) {
                    i2[0] = JavaThreadPoolFactory.getThreadId();
                    i2.notify();
                }
            }
        }.start();
        synchronized (i2) {
            while (i2[0] == null) {
                i2.wait();
            }
        }
        assure(!i1.equals(i2[0]));
    }
}
