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

/* This test has to detect whether the spawned client process hangs, which can
 * not be done reliably.  As an approximation, it waits for 10 sec and considers
 * the process hanging if it has not terminated by then.
 */
public final class StopMessageDispatcherTest extends ComplexTestCase {
    public StopMessageDispatcherTest() {}

    public String[] getTestMethodNames() {
        return new String[] { "test" };
    }

    public void test() throws Exception {
        assure(
            "test",
            new TestBed().execute(new Provider(), false, Client.class, 10000));
    }

    public static final class Client extends TestBed.Client {
        public static void main(String[] args) {
            new Client().execute();
        }

        protected boolean run(XComponentContext context) throws Throwable {
            XTest test = UnoRuntime.queryInterface(
                XTest.class, getBridge(context).getInstance("Test"));
            Thread[] threads = new Thread[101];
            int n = Thread.enumerate(threads);
            if (n > 100) {
                System.err.println("ERROR: too many threads");
                return false;
            }
            boolean stopped = false;
            for (int i = 0; i < n; ++i) {
                if (threads[i].getName().equals("MessageDispatcher")) {
                    threads[i].stop();
                    stopped = true;
                    break;
                }
            }
            if (!stopped) {
                System.err.println("ERROR: thread not found");
                return false;
            }
            try {
                test.call();
                System.err.println("ERROR: no DisposedException");
                return false;
            } catch (DisposedException e) {
                return true;
            }
        }

        private Client() {}
    }

    private static final class Provider implements XInstanceProvider {
        public Object getInstance(String instanceName) {
            return new XTest() {
                    public void call() {}
                };
        }
    }

    public interface XTest extends XInterface {
        void call();

        TypeInfo[] UNOTYPEINFO = { new MethodTypeInfo("call", 0, 0) };
    }
}
