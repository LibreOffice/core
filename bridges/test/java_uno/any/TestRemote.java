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

package test.java_uno.any;

import com.sun.star.bridge.XInstanceProvider;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import test.lib.TestBed;

//"any" is a reserved word in IDL, so we need to use a different packagename
import test.java_uno.anytest.*;

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
