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

import java.util.Arrays;
import org.junit.Test;
import static org.junit.Assert.*;

public final class ThreadId_Test {
    @Test public void test() {
        ThreadId i1 = ThreadId.createFresh();
        assertTrue(i1.equals(i1));
        assertFalse(i1.equals(null));
        assertFalse(i1.equals(new Object()));
        assertEquals(i1.hashCode(), i1.hashCode());
        byte[] i1bytes = i1.getBytes();
        assertNotNull(i1bytes);
        assertTrue(i1bytes.length >= 5);
        assertEquals('j', i1bytes[0]);
        assertEquals('a', i1bytes[1]);
        assertEquals('v', i1bytes[2]);
        assertEquals('a', i1bytes[3]);
        assertEquals(':', i1bytes[4]);
        assertArrayEquals(i1bytes, i1.getBytes());

        ThreadId i2 = ThreadId.createFresh();
        assertFalse(i1.equals(i2));
        assertFalse(i2.equals(i1));
        assertFalse(Arrays.equals(i1bytes, i2.getBytes()));

        ThreadId i3 = new ThreadId(i1bytes);
        assertTrue(i3.equals(i1));
        assertFalse(i3.equals(i2));
        assertTrue(i1.equals(i3));
        assertEquals(i1.hashCode(), i3.hashCode());
        assertArrayEquals(i1bytes, i3.getBytes());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
