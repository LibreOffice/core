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



package com.sun.star.uno;

import com.sun.star.comp.connections.PipedConnection;
import complexlib.ComplexTestCase;
import util.WaitUnreachable;

public final class UnoRuntime_EnvironmentTest extends ComplexTestCase {
    public String getTestObjectName() {
        return getClass().getName();
    }

    public String[] getTestMethodNames() {
        return new String[] { "test_getEnvironment", "test_getBridge" };
    }

    public void test_getEnvironment() throws java.lang.Exception {
        Object o1 = new Object();
        Object o2 = new Object();

        // get two environments with different contexts
        WaitUnreachable java_environment1 = new WaitUnreachable(
            UnoRuntime.getEnvironment("java", o1));
        WaitUnreachable java_environment2 = new WaitUnreachable(
            UnoRuntime.getEnvironment("java", o2));

        // ensure that the environments are different
        assure("", java_environment1.get() != java_environment2.get());

        // test if we get the same environment when we reget it
        assure("",
               UnoRuntime.areSame(java_environment1.get(),
                                  UnoRuntime.getEnvironment("java", o1)));
        assure("",
               UnoRuntime.areSame(java_environment2.get(),
                                  UnoRuntime.getEnvironment("java", o2)));

        // drop the environments and wait until they are gc
        java_environment1.waitUnreachable();
        java_environment2.waitUnreachable();
    }

    public void test_getBridge() throws java.lang.Exception {
        PipedConnection conn = new PipedConnection(new Object[0]);
        new PipedConnection(new Object[] { conn });

        // get a bridge
        IBridge iBridge = UnoRuntime.getBridgeByName(
            "java", null, "remote", "testname",
            new Object[] { "urp", conn, null });

        // reget the bridge, it must be the same as above
        IBridge iBridge_tmp = UnoRuntime.getBridgeByName(
            "java", null, "remote", "testname",
            new Object[] { "urp", conn, null });
        assure("", UnoRuntime.areSame(iBridge_tmp, iBridge));

        // dispose the bridge, this removes the entry from the runtime
        iBridge.dispose();

        conn = new PipedConnection(new Object[0]);
        new PipedConnection(new Object[] { conn });

        // reget the bridge, it must be a different one
        iBridge_tmp = UnoRuntime.getBridgeByName(
            "java", null, "remote", "testname",
            new Object[]{ "urp", conn, null });
        assure("", !UnoRuntime.areSame(iBridge_tmp, iBridge));
    }
}
