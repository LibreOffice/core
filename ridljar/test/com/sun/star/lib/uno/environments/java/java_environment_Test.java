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

package com.sun.star.lib.uno.environments.java;

import com.sun.star.uno.Type;
import com.sun.star.uno.XInterface;
import org.junit.Test;
import static org.junit.Assert.*;

public final class java_environment_Test {
    @Test public void test() {
        java_environment env = new java_environment(null);

        Object obj = Integer.valueOf(3);
        String[] oid = new String[1];

        Object obj2 = env.registerInterface(obj, oid,
                                            new Type(XInterface.class));
        Object obj3 = env.registerInterface(obj, oid,
                                            new Type(XInterface.class));
        // Register ordinary interface twice:
        assertSame(obj, obj2);
        assertSame(obj, obj3);

        // Ask for registered interface:
        assertSame(
            obj,
            env.getRegisteredInterface(oid[0], new Type(XInterface.class)));

        env.revokeInterface(oid[0], new Type(XInterface.class));
        env.revokeInterface(oid[0], new Type(XInterface.class));
        // Revoke interface:
        assertNull(
            env.getRegisteredInterface(oid[0], new Type(XInterface.class)));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
