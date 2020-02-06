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

import org.junit.Test;
import static org.junit.Assert.*;

public final class JavaThreadPoolFactory_Test {
    @Test public void test() throws InterruptedException {
        ThreadId i1 = JavaThreadPoolFactory.getThreadId();
        assertEquals(i1, JavaThreadPoolFactory.getThreadId());
        final ThreadId[] i2 = new ThreadId[1];
        new Thread() {
            @Override
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
        assertFalse(i1.equals(i2[0]));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
