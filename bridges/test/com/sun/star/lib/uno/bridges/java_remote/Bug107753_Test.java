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

package com.sun.star.lib.uno.bridges.java_remote;

import com.sun.star.bridge.XInstanceProvider;
import com.sun.star.lib.TestBed;
import com.sun.star.lib.uno.typeinfo.MethodTypeInfo;
import com.sun.star.lib.uno.typeinfo.TypeInfo;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.XInterface;
import complexlib.ComplexTestCase;

/**
 * Test case for bug #107753#.
 *
 * <p>Bug #107753# "Java UNO: Proxies should implement intuitive semantics of
 * equals and hashCode" requests that two proxies are equal iff they represent
 * the same UNO object.  This implies that if two proxies repsent the same UNO
 * object, they must have the same hash code.</p>
 */
public final class Bug107753_Test extends ComplexTestCase {
    public String getTestObjectName() {
        return getClass().getName();
    }

    public String[] getTestMethodNames() {
        return new String[] { "test" };
    }

    public void test() throws Exception {
        TestBed t = new TestBed();
        assure("test", t.execute(new Provider(t), false, Client.class, 0));
    }

    public static final class Client extends TestBed.Client {
        public static void main(String[] args) {
            new Client().execute();
        }

        protected boolean run(XComponentContext context) throws Throwable {
            boolean success = true;
            XTransport transport = UnoRuntime.queryInterface(
                XTransport.class, getBridge(context).getInstance("Transport"));

            Object obj1a = new XType1() {};
            XType1 obj1b = UnoRuntime.queryInterface(XType1.class, obj1a);
            success &= test("obj1a == obj1b", obj1a == obj1b);

            Object obj2a = new XType2() {};
            XType2 obj2b = UnoRuntime.queryInterface(XType2.class, obj2a);
            success &= test("obj2a == obj2b", obj2a == obj2b);

            Object obj3a = transport.getType1();
            XType1 obj3b = UnoRuntime.queryInterface(XType1.class, obj3a);
            success &= test(
                "obj3a != obj3b; only meaningful as long as different proxy"
                + " instances are used for different UNO interfaces of one UNO"
                + " object",
                obj3a != obj3b);

            Object obj4a = transport.getType2();
            XType2 obj4b = UnoRuntime.queryInterface(XType2.class, obj4a);
            success &= test(
                "obj4a != obj4b; only meaningful as long as different proxy"
                + " instances are used for different UNO interfaces of one UNO"
                + " object",
                obj4a != obj4b);

            success &= test("UnoRuntime.areSame(null, null)",
                            UnoRuntime.areSame(null, null));
            success &= test("!UnoRuntime.areSame(null, obj1a)",
                            !UnoRuntime.areSame(null, obj1a));
            success &= test("!UnoRuntime.areSame(null, obj1b)",
                            !UnoRuntime.areSame(null, obj1b));
            success &= test("!UnoRuntime.areSame(null, obj2a)",
                            !UnoRuntime.areSame(null, obj2a));
            success &= test("!UnoRuntime.areSame(null, obj2b)",
                            !UnoRuntime.areSame(null, obj2b));
            success &= test("!UnoRuntime.areSame(null, obj3a)",
                            !UnoRuntime.areSame(null, obj3a));
            success &= test("!UnoRuntime.areSame(null, obj3b)",
                            !UnoRuntime.areSame(null, obj3b));
            success &= test("!UnoRuntime.areSame(null, obj4a)",
                            !UnoRuntime.areSame(null, obj4a));
            success &= test("!UnoRuntime.areSame(null, obj4b)",
                            !UnoRuntime.areSame(null, obj4b));

            success &= test("!obj1a.equals(null)", !obj1a.equals(null));
            success &= test("!UnoRuntime.areSame(obj1a, null)",
                            !UnoRuntime.areSame(obj1a, null));
            success &= test("obj1a.equals(obj1a)", obj1a.equals(obj1a));
            success &= test("UnoRuntime.areSame(obj1a, obj1a)",
                            UnoRuntime.areSame(obj1a, obj1a));
            success &= test("obj1a.equals(obj1b)", obj1a.equals(obj1b));
            success &= test("UnoRuntime.areSame(obj1a, obj1b)",
                            UnoRuntime.areSame(obj1a, obj1b));
            success &= test("!obj1a.equals(obj2a)", !obj1a.equals(obj2a));
            success &= test("!UnoRuntime.areSame(obj1a, obj2a)",
                            !UnoRuntime.areSame(obj1a, obj2a));
            success &= test("!obj1a.equals(obj2b)", !obj1a.equals(obj2b));
            success &= test("!UnoRuntime.areSame(obj1a, obj2b)",
                            !UnoRuntime.areSame(obj1a, obj2b));
            success &= test("!obj1a.equals(obj3a)", !obj1a.equals(obj3a));
            success &= test("!UnoRuntime.areSame(obj1a, obj3a)",
                            !UnoRuntime.areSame(obj1a, obj3a));
            success &= test("!obj1a.equals(obj3b)", !obj1a.equals(obj3b));
            success &= test("!UnoRuntime.areSame(obj1a, obj3b)",
                            !UnoRuntime.areSame(obj1a, obj3b));
            success &= test("!obj1a.equals(obj4a)", !obj1a.equals(obj4a));
            success &= test("!UnoRuntime.areSame(obj1a, obj4a)",
                            !UnoRuntime.areSame(obj1a, obj4a));
            success &= test("!obj1a.equals(obj4b)", !obj1a.equals(obj4b));
            success &= test("!UnoRuntime.areSame(obj1a, obj4b)",
                            !UnoRuntime.areSame(obj1a, obj4b));

            success &= test("!obj1b.equals(null)", !obj1b.equals(null));
            success &= test("!UnoRuntime.areSame(obj1b, null)",
                            !UnoRuntime.areSame(obj1b, null));
            success &= test("obj1b.equals(obj1a)", obj1b.equals(obj1a));
            success &= test("UnoRuntime.areSame(obj1b, obj1a)",
                            UnoRuntime.areSame(obj1b, obj1a));
            success &= test("obj1b.equals(obj1b)", obj1b.equals(obj1b));
            success &= test("UnoRuntime.areSame(obj1b, obj1b)",
                            UnoRuntime.areSame(obj1b, obj1b));
            success &= test("!obj1b.equals(obj2a)", !obj1b.equals(obj2a));
            success &= test("!UnoRuntime.areSame(obj1b, obj2a)",
                            !UnoRuntime.areSame(obj1b, obj2a));
            success &= test("!obj1b.equals(obj2b)", !obj1b.equals(obj2b));
            success &= test("!UnoRuntime.areSame(obj1b, obj2b)",
                            !UnoRuntime.areSame(obj1b, obj2b));
            success &= test("!obj1b.equals(obj3a)", !obj1b.equals(obj3a));
            success &= test("!UnoRuntime.areSame(obj1b, obj3a)",
                            !UnoRuntime.areSame(obj1b, obj3a));
            success &= test("!obj1b.equals(obj3b)", !obj1b.equals(obj3b));
            success &= test("!UnoRuntime.areSame(obj1b, obj3b)",
                            !UnoRuntime.areSame(obj1b, obj3b));
            success &= test("!obj1b.equals(obj4a)", !obj1b.equals(obj4a));
            success &= test("!UnoRuntime.areSame(obj1b, obj4a)",
                            !UnoRuntime.areSame(obj1b, obj4a));
            success &= test("!obj1b.equals(obj4b)", !obj1b.equals(obj4b));
            success &= test("!UnoRuntime.areSame(obj1b, obj4b)",
                            !UnoRuntime.areSame(obj1b, obj4b));

            success &= test("!obj2a.equals(null)", !obj2a.equals(null));
            success &= test("!UnoRuntime.areSame(obj2a, null)",
                            !UnoRuntime.areSame(obj2a, null));
            success &= test("!obj2a.equals(obj1a)", !obj2a.equals(obj1a));
            success &= test("!UnoRuntime.areSame(obj2a, obj1a)",
                            !UnoRuntime.areSame(obj2a, obj1a));
            success &= test("!obj2a.equals(obj1b)", !obj2a.equals(obj1b));
            success &= test("!UnoRuntime.areSame(obj2a, obj1b)",
                            !UnoRuntime.areSame(obj2a, obj1b));
            success &= test("obj2a.equals(obj2a)", obj2a.equals(obj2a));
            success &= test("UnoRuntime.areSame(obj2a, obj2a)",
                            UnoRuntime.areSame(obj2a, obj2a));
            success &= test("obj2a.equals(obj2b)", obj2a.equals(obj2b));
            success &= test("UnoRuntime.areSame(obj2a, obj2b)",
                            UnoRuntime.areSame(obj2a, obj2b));
            success &= test("!obj2a.equals(obj3a)", !obj2a.equals(obj3a));
            success &= test("!UnoRuntime.areSame(obj2a, obj3a)",
                            !UnoRuntime.areSame(obj2a, obj3a));
            success &= test("!obj2a.equals(obj3b)", !obj2a.equals(obj3b));
            success &= test("!UnoRuntime.areSame(obj2a, obj3b)",
                            !UnoRuntime.areSame(obj2a, obj3b));
            success &= test("!obj2a.equals(obj4a)", !obj2a.equals(obj4a));
            success &= test("!UnoRuntime.areSame(obj2a, obj4a)",
                            !UnoRuntime.areSame(obj2a, obj4a));
            success &= test("!obj2a.equals(obj4b)", !obj2a.equals(obj4b));
            success &= test("!UnoRuntime.areSame(obj2a, obj4b)",
                            !UnoRuntime.areSame(obj2a, obj4b));

            success &= test("!obj2b.equals(null)", !obj2b.equals(null));
            success &= test("!UnoRuntime.areSame(obj2b, null)",
                            !UnoRuntime.areSame(obj2b, null));
            success &= test("!obj2b.equals(obj1a)", !obj2b.equals(obj1a));
            success &= test("!UnoRuntime.areSame(obj2b, obj1a)",
                            !UnoRuntime.areSame(obj2b, obj1a));
            success &= test("!obj2b.equals(obj1b)", !obj2b.equals(obj1b));
            success &= test("!UnoRuntime.areSame(obj2b, obj1b)",
                            !UnoRuntime.areSame(obj2b, obj1b));
            success &= test("obj2b.equals(obj2a)", obj2b.equals(obj2a));
            success &= test("UnoRuntime.areSame(obj2b, obj2a)",
                            UnoRuntime.areSame(obj2b, obj2a));
            success &= test("obj2b.equals(obj2b)", obj2b.equals(obj2b));
            success &= test("UnoRuntime.areSame(obj2b, obj2b)",
                            UnoRuntime.areSame(obj2b, obj2b));
            success &= test("!obj2b.equals(obj3a)", !obj2b.equals(obj3a));
            success &= test("!UnoRuntime.areSame(obj2b, obj3a)",
                            !UnoRuntime.areSame(obj2b, obj3a));
            success &= test("!obj2b.equals(obj3b)", !obj2b.equals(obj3b));
            success &= test("!UnoRuntime.areSame(obj2b, obj3b)",
                            !UnoRuntime.areSame(obj2b, obj3b));
            success &= test("!obj2b.equals(obj4a)", !obj2b.equals(obj4a));
            success &= test("!UnoRuntime.areSame(obj2b, obj4a)",
                            !UnoRuntime.areSame(obj2b, obj4a));
            success &= test("!obj2b.equals(obj4b)", !obj2b.equals(obj4b));
            success &= test("!UnoRuntime.areSame(obj2b, obj4b)",
                            !UnoRuntime.areSame(obj2b, obj4b));

            success &= test("!obj3a.equals(null)", !obj3a.equals(null));
            success &= test("!UnoRuntime.areSame(obj3a, null)",
                            !UnoRuntime.areSame(obj3a, null));
            success &= test("!obj3a.equals(obj1a)", !obj3a.equals(obj1a));
            success &= test("!UnoRuntime.areSame(obj3a, obj1a)",
                            !UnoRuntime.areSame(obj3a, obj1a));
            success &= test("!obj3a.equals(obj1b)", !obj3a.equals(obj1b));
            success &= test("!UnoRuntime.areSame(obj3a, obj1b)",
                            !UnoRuntime.areSame(obj3a, obj1b));
            success &= test("!obj3a.equals(obj2a)", !obj3a.equals(obj2a));
            success &= test("!UnoRuntime.areSame(obj3a, obj2a)",
                            !UnoRuntime.areSame(obj3a, obj2a));
            success &= test("!obj3a.equals(obj2b)", !obj3a.equals(obj2b));
            success &= test("!UnoRuntime.areSame(obj3a, obj2b)",
                            !UnoRuntime.areSame(obj3a, obj2b));
            success &= test("obj3a.equals(obj3a)", obj3a.equals(obj3a));
            success &= test("UnoRuntime.areSame(obj3a, obj3a)",
                            UnoRuntime.areSame(obj3a, obj3a));
            success &= test("obj3a.equals(obj3b)", obj3a.equals(obj3b));
            success &= test("UnoRuntime.areSame(obj3a, obj3b)",
                            UnoRuntime.areSame(obj3a, obj3b));
            success &= test("!obj3a.equals(obj4a)", !obj3a.equals(obj4a));
            success &= test("!UnoRuntime.areSame(obj3a, obj4a)",
                            !UnoRuntime.areSame(obj3a, obj4a));
            success &= test("!obj3a.equals(obj4b)", !obj3a.equals(obj4b));
            success &= test("!UnoRuntime.areSame(obj3a, obj4b)",
                            !UnoRuntime.areSame(obj3a, obj4b));

            success &= test("!obj3b.equals(null)", !obj3b.equals(null));
            success &= test("!UnoRuntime.areSame(obj3b, null)",
                            !UnoRuntime.areSame(obj3b, null));
            success &= test("!obj3b.equals(obj1a)", !obj3b.equals(obj1a));
            success &= test("!UnoRuntime.areSame(obj3b, obj1a)",
                            !UnoRuntime.areSame(obj3b, obj1a));
            success &= test("!obj3b.equals(obj1b)", !obj3b.equals(obj1b));
            success &= test("!UnoRuntime.areSame(obj3b, obj1b)",
                            !UnoRuntime.areSame(obj3b, obj1b));
            success &= test("!obj3b.equals(obj2a)", !obj3b.equals(obj2a));
            success &= test("!UnoRuntime.areSame(obj3b, obj2a)",
                            !UnoRuntime.areSame(obj3b, obj2a));
            success &= test("!obj3b.equals(obj2b)", !obj3b.equals(obj2b));
            success &= test("!UnoRuntime.areSame(obj3b, obj2b)",
                            !UnoRuntime.areSame(obj3b, obj2b));
            success &= test("obj3b.equals(obj3a)", obj3b.equals(obj3a));
            success &= test("UnoRuntime.areSame(obj3b, obj3a)",
                            UnoRuntime.areSame(obj3b, obj3a));
            success &= test("obj3b.equals(obj3b)", obj3b.equals(obj3b));
            success &= test("UnoRuntime.areSame(obj3b, obj3b)",
                            UnoRuntime.areSame(obj3b, obj3b));
            success &= test("!obj3b.equals(obj4a)", !obj3b.equals(obj4a));
            success &= test("!UnoRuntime.areSame(obj3b, obj4a)",
                            !UnoRuntime.areSame(obj3b, obj4a));
            success &= test("!obj3b.equals(obj4b)", !obj3b.equals(obj4b));
            success &= test("!UnoRuntime.areSame(obj3b, obj4b)",
                            !UnoRuntime.areSame(obj3b, obj4b));

            success &= test("!obj4a.equals(null)", !obj4a.equals(null));
            success &= test("!UnoRuntime.areSame(obj4a, null)",
                            !UnoRuntime.areSame(obj4a, null));
            success &= test("!obj4a.equals(obj1a)", !obj4a.equals(obj1a));
            success &= test("!UnoRuntime.areSame(obj4a, obj1a)",
                            !UnoRuntime.areSame(obj4a, obj1a));
            success &= test("!obj4a.equals(obj1b)", !obj4a.equals(obj1b));
            success &= test("!UnoRuntime.areSame(obj4a, obj1b)",
                            !UnoRuntime.areSame(obj4a, obj1b));
            success &= test("!obj4a.equals(obj2a)", !obj4a.equals(obj2a));
            success &= test("!UnoRuntime.areSame(obj4a, obj2a)",
                            !UnoRuntime.areSame(obj4a, obj2a));
            success &= test("!obj4a.equals(obj2b)", !obj4a.equals(obj2b));
            success &= test("!UnoRuntime.areSame(obj4a, obj2b)",
                            !UnoRuntime.areSame(obj4a, obj2b));
            success &= test("!obj4a.equals(obj3a)", !obj4a.equals(obj3a));
            success &= test("!UnoRuntime.areSame(obj4a, obj3a)",
                            !UnoRuntime.areSame(obj4a, obj3a));
            success &= test("!obj4a.equals(obj3b)", !obj4a.equals(obj3b));
            success &= test("!UnoRuntime.areSame(obj4a, obj3b)",
                            !UnoRuntime.areSame(obj4a, obj3b));
            success &= test("obj4a.equals(obj4a)", obj4a.equals(obj4a));
            success &= test("UnoRuntime.areSame(obj4a, obj4a)",
                            UnoRuntime.areSame(obj4a, obj4a));
            success &= test("obj4a.equals(obj4b)", obj4a.equals(obj4b));
            success &= test("UnoRuntime.areSame(obj4a, obj4b)",
                            UnoRuntime.areSame(obj4a, obj4b));

            success &= test("!obj4b.equals(null)", !obj4b.equals(null));
            success &= test("!UnoRuntime.areSame(obj4b, null)",
                            !UnoRuntime.areSame(obj4b, null));
            success &= test("!obj4b.equals(obj1a)", !obj4b.equals(obj1a));
            success &= test("!UnoRuntime.areSame(obj4b, obj1a)",
                            !UnoRuntime.areSame(obj4b, obj1a));
            success &= test("!obj4b.equals(obj1b)", !obj4b.equals(obj1b));
            success &= test("!UnoRuntime.areSame(obj4b, obj1b)",
                            !UnoRuntime.areSame(obj4b, obj1b));
            success &= test("!obj4b.equals(obj2a)", !obj4b.equals(obj2a));
            success &= test("!UnoRuntime.areSame(obj4b, obj2a)",
                            !UnoRuntime.areSame(obj4b, obj2a));
            success &= test("!obj4b.equals(obj2b)", !obj4b.equals(obj2b));
            success &= test("!UnoRuntime.areSame(obj4b, obj2b)",
                            !UnoRuntime.areSame(obj4b, obj2b));
            success &= test("!obj4b.equals(obj3a)", !obj4b.equals(obj3a));
            success &= test("!UnoRuntime.areSame(obj4b, obj3a)",
                            !UnoRuntime.areSame(obj4b, obj3a));
            success &= test("!obj4b.equals(obj3b)", !obj4b.equals(obj3b));
            success &= test("!UnoRuntime.areSame(obj4b, obj3b)",
                            !UnoRuntime.areSame(obj4b, obj3b));
            success &= test("obj4b.equals(obj4a)", obj4b.equals(obj4a));
            success &= test("UnoRuntime.areSame(obj4b, obj4a)",
                            UnoRuntime.areSame(obj4b, obj4a));
            success &= test("obj4b.equals(obj4b)", obj4b.equals(obj4b));
            success &= test("UnoRuntime.areSame(obj4b, obj4b)",
                            UnoRuntime.areSame(obj4b, obj4b));

            success &= test("obj1a.hashCode() == obj1b.hashCode()",
                            obj1a.hashCode() == obj1b.hashCode());
            success &= test("obj2a.hashCode() == obj2b.hashCode()",
                            obj2a.hashCode() == obj2b.hashCode());
            success &= test("obj3a.hashCode() == obj3b.hashCode()",
                            obj3a.hashCode() == obj3b.hashCode());
            success &= test("obj4a.hashCode() == obj4b.hashCode()",
                            obj4a.hashCode() == obj4b.hashCode());

            return success;
        }

        private static boolean test(String message, boolean condition) {
            if (!condition) {
                System.err.println("Failed: " + message);
            }
            return condition;
        }
    }

    private static final class Provider implements XInstanceProvider {
        public Provider(TestBed testBed) {
            this.testBed = testBed;
        }

        public Object getInstance(String instanceName) {
            return new XTransport() {
                    public Object getType1() {
                        return new XType1() {};
                    }

                    public Object getType2() {
                        return new XType2() {};
                    }
                };
        }

        private final TestBed testBed;
    }

    public interface XTransport extends XInterface {
        Object getType1();

        Object getType2();

        TypeInfo[] UNOTYPEINFO = { new MethodTypeInfo("getType1", 0, 0),
                                   new MethodTypeInfo("getType2", 1, 0) };
    }

    public interface XType1 extends XInterface {
        TypeInfo[] UNOTYPEINFO = null;
    }

    public interface XType2 extends XInterface {
        TypeInfo[] UNOTYPEINFO = null;
    }
}
