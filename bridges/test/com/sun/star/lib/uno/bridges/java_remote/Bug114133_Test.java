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



package com.sun.star.lib.uno.bridges.javaremote;

import com.sun.star.bridge.XInstanceProvider;
import com.sun.star.lib.TestBed;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.XInterface;
import complexlib.ComplexTestCase;
import util.WaitUnreachable;

/**
 * Test case for bug #114133#.
 *
 * <p>Bug #114133# "Java UNO: UnoRuntime.getBride and disposed bridges."  The
 * client calls UnoRuntime.getBridge to get a bridge to the server, uses the
 * bridge, waits until it terminates itself (when all bridged objects have been
 * garbage-collected), then calls UnoRuntime.getBridge again.  This must return
 * a fresh, unterminated bridge.</p>
 */
public final class Bug114133_Test extends ComplexTestCase {
    public String[] getTestMethodNames() {
        return new String[] { "test" };
    }

    public void test() throws Exception {
        assure(
            "test",
            new TestBed().execute(new Provider(), false, Client.class, 0));
    }

    public static final class Client extends TestBed.Client {
        public static void main(String[] args) {
            new Client().execute();
        }

        protected boolean run(XComponentContext context) throws Throwable {
            new WaitUnreachable(getBridge(context).getInstance("Test")).
                waitUnreachable();
            new WaitUnreachable(getBridge(context).getInstance("Test")).
                waitUnreachable();
            return true;
        }
    }

    private static final class Provider implements XInstanceProvider {
        public Object getInstance(String instanceName) {
            return new XInterface() {};
        }
    }
}
