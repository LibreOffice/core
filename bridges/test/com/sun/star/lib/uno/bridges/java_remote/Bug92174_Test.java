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
import com.sun.star.lib.uno.typeinfo.MethodTypeInfo;
import com.sun.star.lib.uno.typeinfo.TypeInfo;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.XInterface;
import complexlib.ComplexTestCase;

public final class Bug92174_Test extends ComplexTestCase {
    public String[] getTestMethodNames() {
        return new String[] { "test" };
    }

    public void test() throws Exception {
        assure("test",
               new TestBed().execute(new Provider(), false, Client.class, 0));
    }

    public static final class Client extends TestBed.Client {
        public static void main(String[] args) {
            new Client().execute();
        }

        protected boolean run(XComponentContext context) throws Throwable {
            XTransport t = UnoRuntime.queryInterface(
                XTransport.class, getBridge(context).getInstance("Transport"));
            t.setDerived(new XDerived() {
                    public void fn() {}
                });
            t.getBase().fn();
            return true;
        }
    }

    private static final class Provider implements XInstanceProvider {
        public Object getInstance(String instanceName) {
            return new XTransport() {
                    public XBase getBase() {
                        return derived;
                    }

                    public synchronized void setDerived(XDerived derived) {
                        this.derived = derived;
                    }

                    private XDerived derived = null;
                };
        }
    }

    public interface XBase extends XInterface {
        void fn();

        TypeInfo[] UNOTYPEINFO = { new MethodTypeInfo("fn", 0, 0) };
    }

    public interface XDerived extends XBase {
        TypeInfo[] UNOTYPEINFO = null;
    }

    public interface XTransport extends XInterface {
        XBase getBase();

        void setDerived(XDerived derived);

        TypeInfo[] UNOTYPEINFO = { new MethodTypeInfo("getBase", 0, 0),
                                   new MethodTypeInfo("setDerived", 1, 0) };
    }
}
