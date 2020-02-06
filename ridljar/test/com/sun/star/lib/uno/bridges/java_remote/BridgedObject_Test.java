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

package com.sun.star.lib.uno.bridges.java_remote;

import com.sun.star.bridge.XBridge;
import com.sun.star.uno.Type;
import com.sun.star.uno.XInterface;
import org.junit.Test;
import static org.junit.Assert.*;

public final class BridgedObject_Test {
    @Test public void test() {
        RequestHandler handler = new RequestHandler() {
                public Object sendRequest(
                    String oid, Type type, String operation, Object[] args)
                {
                    return null;
                }
            };
        XBridge bridge1 = new TestBridge();
        ProxyFactory factory1 = new ProxyFactory(handler, bridge1);
        XBridge bridge2 = new TestBridge();
        ProxyFactory factory2 = new ProxyFactory(handler, bridge2);
        Object object0 = new Object();
        Object object1 = factory1.create("", new Type(XInterface.class));
        Object object2 = factory2.create("", new Type(XInterface.class));
        assertNull(BridgedObject.getBridge(object0));
        assertSame(bridge1, BridgedObject.getBridge(object1));
        assertSame(bridge2, BridgedObject.getBridge(object2));
    }

    private static final class TestBridge implements XBridge {
        public Object getInstance(String instanceName) {
            return null;
        }

        public String getName() {
            return null;
        }

        public String getDescription() {
            return null;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
