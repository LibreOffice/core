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

package com.sun.star.lib.uno.bridges.java_remote;

import com.sun.star.bridge.XBridge;
import com.sun.star.uno.Type;
import com.sun.star.uno.XInterface;
import complexlib.ComplexTestCase;

public final class BridgedObject_Test extends ComplexTestCase {
    public String[] getTestMethodNames() {
        return new String[] { "test" };
    }

    public void test() {
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
        assure(BridgedObject.getBridge(object0) == null);
        assure(BridgedObject.getBridge(object1) == bridge1);
        assure(BridgedObject.getBridge(object2) == bridge2);
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
