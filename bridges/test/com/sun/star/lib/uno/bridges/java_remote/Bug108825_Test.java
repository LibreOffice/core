/*************************************************************************
 *
 *  $RCSfile: Bug108825_Test.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 03:02:33 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package com.sun.star.lib.uno.bridges.javaremote;

import com.sun.star.bridge.XBridge;
import com.sun.star.bridge.XInstanceProvider;
import com.sun.star.lib.TestBed;
import com.sun.star.lib.uno.typeinfo.MethodTypeInfo;
import com.sun.star.lib.uno.typeinfo.TypeInfo;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import complexlib.ComplexTestCase;

/**
 * Test case for bug #108825#.
 *
 * <p>Bug #108825# "Java UNO Remote Bridge: Mapped-out Objects Not Held" shows
 * that local objects that are mapped out via a remote bridge, but not held
 * locally, might be garbage collected while there are still remote references
 * to them.  This test is not guaranteed to always work reliably, see comment in
 * the code.</p>
 */
public final class Bug108825_Test extends ComplexTestCase {
    public String getTestObjectName() {
        return getClass().getName();
    }

    public String[] getTestMethodNames() {
        return new String[] { "test" };
    }

    public void test() throws Exception {
        TestBed t = new TestBed();
        assure("test", t.execute(new Provider(t), true, Client.class, 0));
    }

    public static final class Client extends TestBed.Client {
        public static void main(String[] args) {
            new Client().execute();
        }

        protected boolean run(XBridge bridge) throws Throwable {
            XTest test = (XTest) UnoRuntime.queryInterface(
                XTest.class, bridge.getInstance("Test"));
            // Send the XObject that is held on the server side amidst two
            // dummies that are not held on the server side; then wait for the
            // dummies to be garbage collected, hoping that the XObject, if it
            // is erroneously not held on the client side, will be garbage
            // collected, too.  Obviously, this is not guaranteed to always work
            // (the VM might chose not to garbage collect the dummies, hanging
            // the test forever; or the VM might chose to garbage collect the
            // dummies but not the XObject, making the test pass erroneously).
            test.offer(new Dummy(), new XObject() { public void call() {} },
                       new Dummy());
            System.out.println("Client waiting for garbage collection...");
            for (;;) {
                synchronized (lock) {
                    if (finalizedCount == 2) {
                        break;
                    }
                }
                test.remoteGc();
                gc();
            }
            System.out.println("Client garbage collection done.");
            test.notification();
            return true;
        }

        private final class Dummy implements XDummy {
            protected void finalize() {
                synchronized (lock) {
                    ++finalizedCount;
                }
            }
        }

        private final Object lock = new Object();
        private int finalizedCount = 0;
    }

    // Make it as likely as possible that the VM reclaims all garbage:
    private static void gc() {
        System.gc();
        System.runFinalization();
        byte[] garbage = new byte[1024 * 1024];
    }

    private static final class Provider implements XInstanceProvider {
        public Provider(TestBed testBed) {
            this.testBed = testBed;
        }

        public Object getInstance(String instanceName) {
            return new XTest() {
                    public void offer(XDummy dummy1, XObject obj, XDummy dummy2)
                    {
                        this.obj = obj;
                    }

                    public void remoteGc() {
                        gc();
                    }

                    public void notification() {
                        obj.call();
                        testBed.serverDone(true);
                    }

                    private XObject obj;
                };
        }

        private final TestBed testBed;
    }

    public interface XDummy extends XInterface {
        TypeInfo[] UNOTYPEINFO = null;
    }

    public interface XObject extends XInterface {
        void call();

        TypeInfo[] UNOTYPEINFO = { new MethodTypeInfo("call", 0, 0) };
    }

    public interface XTest extends XInterface {
        void offer(XDummy dummy1, XObject obj, XDummy dummy2);

        void remoteGc();

        void notification();

        TypeInfo[] UNOTYPEINFO = { new MethodTypeInfo("offer", 0, 0),
                                   new MethodTypeInfo("remoteGc", 1, 0),
                                   new MethodTypeInfo("notification", 2, 0) };
    }
}
