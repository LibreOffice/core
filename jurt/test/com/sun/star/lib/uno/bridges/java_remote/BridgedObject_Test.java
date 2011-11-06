/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
