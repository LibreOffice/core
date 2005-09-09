/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TestBase.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 12:29:06 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package testtools.servicetests;

import com.sun.star.uno.UnoRuntime;
import complexlib.ComplexTestCase;
import util.WaitUnreachable;

public abstract class TestBase extends ComplexTestCase {
    public final String[] getTestMethodNames() {
        return new String[] { "test" };
    }

    public final void test() throws Exception {
        TestServiceFactory factory = getTestServiceFactory();
        TestService2 t = (TestService2) UnoRuntime.queryInterface(
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
        XTestService3 t3 = (XTestService3) UnoRuntime.queryInterface(
            XTestService3.class, t);
        assure(t3 != null);
        assure(t3.fn3() == 3);
        XTestService4 t4 = (XTestService4) UnoRuntime.queryInterface(
            XTestService4.class, t);
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
