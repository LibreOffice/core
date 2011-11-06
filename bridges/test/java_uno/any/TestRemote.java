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



package test.java_uno.anytest;

import com.sun.star.bridge.XInstanceProvider;
import com.sun.star.lib.TestBed;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;

public final class TestRemote {
    public static void main(String[] args) throws Exception {
        boolean success = new TestBed().execute(
            new Provider(), false, Client.class, 0);
        System.out.println("success? " + success);
        System.exit(success ? 0 : 1);
    }

    public static final class Client extends TestBed.Client {
        public static void main(String[] args) {
            new Client().execute();
        }

        protected boolean run(XComponentContext context) throws Throwable {
            XTransport transport = UnoRuntime.queryInterface(
                XTransport.class, getBridge(context).getInstance("Transport"));
            return TestAny.test(transport, true);
        }
    }

    private static final class Provider implements XInstanceProvider {
        public Object getInstance(String instanceName) {
            return new XTransport() {
                    public Object mapAny(Object any) {
                        return any;
                    }
                };
        }
    }
}
