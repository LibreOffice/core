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
import com.sun.star.lib.uno.typeinfo.MethodTypeInfo;
import com.sun.star.lib.uno.typeinfo.TypeInfo;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.XInterface;
import complexlib.ComplexTestCase;
import test.lib.TestBed;

/**
 * Test case for bug #111153#.
 *
 * <P>Bug #111153# "jni_uno bridge sometimes fails to map objects
 * correctly" describes that mapping a local object out with type XDerived and
 * then mapping it back in with type XBase produces a proxy, instead of
 * short-cutting to the local object.</P>
 */
public final class Bug111153_Test extends ComplexTestCase {
    public String[] getTestMethodNames() {
        return new String[] { "test" };
    }

    public void test() throws Exception {
        assure("test", new TestBed().execute(new Provider(), false,
                                             Client.class, 0));
    }

    public static final class Client extends TestBed.Client {
        public static void main(String[] args) {
            new Client().execute();
        }

        protected boolean run(XComponentContext context) throws Throwable {
            XTransport t = UnoRuntime.queryInterface(
                XTransport.class, getBridge(context).getInstance("Transport"));
            XDerived d = new XDerived() {};
            t.setDerived(d);
            return t.getBase() == d;
        }
    }

    private static final class Provider implements XInstanceProvider {
        public Object getInstance(String instanceName) {
            return new XTransport() {
                    public synchronized void setDerived(XDerived derived) {
                        this.derived = derived;
                    }

                    public synchronized XBase getBase() {
                        return this.derived;
                    }

                    private XDerived derived = null;
                };
        }
    }

    public interface XBase extends XInterface {
        TypeInfo[] UNOTYPEINFO = null;
    }

    public interface XDerived extends XBase {
        TypeInfo[] UNOTYPEINFO = null;
    }

    public interface XTransport extends XInterface {
        void setDerived(XDerived derived);

        XBase getBase();

        TypeInfo[] UNOTYPEINFO = { new MethodTypeInfo("setDerived", 0, 0),
                                   new MethodTypeInfo("getBase", 1, 0) };
    }
}
