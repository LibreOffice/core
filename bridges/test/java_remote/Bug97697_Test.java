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

package test.java_remote;

import com.sun.star.bridge.XInstanceProvider;
import com.sun.star.lang.DisposedException;
import com.sun.star.lib.uno.typeinfo.MethodTypeInfo;
import com.sun.star.lib.uno.typeinfo.TypeInfo;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.XInterface;
import complexlib.ComplexTestCase;
import test.lib.TestBed;

/**
 * Test case for bug #97697#.
 *
 * <p>Bug #97697# "GPF in java-uno bridge in bugdoc scenario" shows that sending
 * a plain <code>Object</code> as an <code>Any</code> over the URP bridge lead
 * to a <code>StackOverflowError</code> on the writer thread, which was silently
 * discarded (and the bridge was not disposed).</p>
 *
 * <p>This test has to detect whether the spawned client process indeed hangs,
 * which can not be done reliably.  As an approximation, it waits for 10 sec and
 * considers the process hanging if it has not completed by then.</p>
 */
public final class Bug97697_Test extends ComplexTestCase {
    public String getTestObjectName() {
        return getClass().getName();
    }

    public String[] getTestMethodNames() {
        return new String[] { "test" };
    }

    public void test() throws Exception {
        TestBed t = new TestBed();
        assure("test", t.execute(new Provider(t), true, Client.class, 10000));
    }

    public static final class Client extends TestBed.Client {
        public static void main(String[] args) {
            new Client().execute();
        }

        protected boolean run(XComponentContext context) throws Throwable {
            XTransport transport = UnoRuntime.queryInterface(
                XTransport.class, getBridge(context).getInstance("Transport"));
            try {
                transport.getAny();
            } catch (DisposedException e) {
                return true;
            }
            return false;
        }
    }

    private static final class Provider implements XInstanceProvider {
        public Provider(TestBed testBed) {
            this.testBed = testBed;
        }

        public Object getInstance(String instanceName) {
            return new XTransport() {
                    public Object getAny() {
                        testBed.serverDone(true);
                        return new Object();
                    }
                };
        }

        private final TestBed testBed;
    }

    public interface XTransport extends XInterface {
        Object getAny();

        TypeInfo[] UNOTYPEINFO = { new MethodTypeInfo("getAny", 0, 0) };
    }
}
