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

package servicetests;

import com.sun.star.uno.UnoRuntime;
import complexlib.ComplexTestCase;
import util.WaitUnreachable;

public abstract class TestBase extends ComplexTestCase {
    public final String[] getTestMethodNames() {
        return new String[] { "test" };
    }

    public final void test() throws Exception {
        TestServiceFactory factory = getTestServiceFactory();
        TestService2 t = UnoRuntime.queryInterface(
            TestService2.class, factory.get());
        assure(t != null);
        assure(UnoRuntime.queryInterface(TestService1.class, t) == t);
        assure(UnoRuntime.queryInterface(XTestService1.class, t) == t);
        assure(UnoRuntime.queryInterface(XTestService2.class, t) == t);
        assure(t.fn1() == 1);
        assure(t.getProp1() == 1);
        t.setProp1(0);
        assure(t.getProp1() == 0);
        assure(t.getProp2() == 2);
        /*try {
            t.getProp3Void();
            failed();
        } catch (VoidPropertyException e) {
        }*/
        assure(t.getProp3Long() == 3);
        /*try {
            t.getProp4None();
            failed();
        } catch (OptionalPropertyException e) {
        }*/
        assure(t.getProp4Long() == 4);
        /*try {
            t.getProp5None();
            failed();
        } catch (OptionalPropertyException e) {
        }
        try {
            t.getProp5Void();
            failed();
        } catch (VoidPropertyException e) {
        }*/
        assure(t.getProp5Long() == 5);
        assure(t.getProp6() == 6);
        /*t.clearProp6();
        try {
            t.getProp6();
            failed();
        } catch (VoidPropertyException e) {
        }*/
        t.setProp6(0);
        assure(t.getProp6() == 0);
        /*try {
            t.getProp7None();
            failed();
        } catch (OptionalPropertyException e) {
        }
        try {
            t.setProp7None(0);
            failed();
        } catch (OptionalPropertyException e) {
        }
        try {
            t.clearProp7None();
            failed();
        } catch (OptionalPropertyException e) {
        }*/
        assure(t.getProp7() == 7);
        /*t.clearProp7();
        try {
            t.getProp7();
            failed();
        } catch (VoidPropertyException e) {
        }*/
        t.setProp7(0);
        assure(t.getProp7() == 0);
        /*try {
            t.getProp8None();
            failed();
        } catch (OptionalPropertyException e) {
        }
        try {
            t.setProp8None(0);
            failed();
        } catch (OptionalPropertyException e) {
        }*/
        assure(t.getProp8Long() == 8);
        t.setProp8Long(0);
        assure(t.getProp8Long() == 0);
        assure(t.fn2() == 2);
        XTestService3 t3 = UnoRuntime.queryInterface(XTestService3.class, t);
        assure(t3 != null);
        assure(t3.fn3() == 3);
        XTestService4 t4 = UnoRuntime.queryInterface(XTestService4.class, t);
        assure(t4 == null);
        WaitUnreachable u = new WaitUnreachable(t);
        t = null;
        WaitUnreachable.ensureFinalization(t3);
        t3 = null;
        WaitUnreachable.ensureFinalization(t4);
        t4 = null;
        u.waitUnreachable();
        factory.dispose();
    }

    protected abstract TestServiceFactory getTestServiceFactory()
        throws Exception;

    protected interface TestServiceFactory {
        Object get() throws Exception;

        void dispose() throws Exception;
    }
}
