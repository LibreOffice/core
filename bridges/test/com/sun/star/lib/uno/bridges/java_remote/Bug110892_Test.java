/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Bug110892_Test.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 13:07:19 $
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

package com.sun.star.lib.uno.bridges.javaremote;

import com.sun.star.bridge.XInstanceProvider;
import com.sun.star.lib.TestBed;
import com.sun.star.lib.uno.typeinfo.MethodTypeInfo;
import com.sun.star.lib.uno.typeinfo.TypeInfo;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.XInterface;
import complexlib.ComplexTestCase;
import util.WaitUnreachable;

/**
 * Test case for bug #110892#.
 *
 * <p>Bug #110892# "Java URP bridge holds objects indefinitely" applies to cases
 * where an object is sent from server to client, then recursively back from
 * client to server.  In such a case, the client should not increment its
 * internal reference count for the object, as the server will never send back a
 * corresponding release message.</p>
 *
 * <p>This test has to detect whether the spawned client process fails to
 * garbage-collect an object, which can not be done reliably.  As an
 * approximation, it waits for 10 sec and considers the process failing if it
 * has not garbage-collected the object by then.</p>
 */
public final class Bug110892_Test extends ComplexTestCase {
    public String[] getTestMethodNames() {
        return new String[] { "test" };
    }

    public void test() throws Exception {
        assure("test",
               new TestBed().execute(new Provider(), false, Client.class,
                                     10000));
    }

    public static final class Client extends TestBed.Client {
        public static void main(String[] args) {
            new Client().execute();
        }

        protected boolean run(XComponentContext context) throws Throwable {
            XTest test = (XTest) UnoRuntime.queryInterface(
                XTest.class, getBridge(context).getInstance("Test"));
            test.start(new ClientObject());
            synchronized (lock) {
                unreachable.waitUnreachable();
            }
            return true;
        }

        private final class ClientObject implements XClientObject {
            public void call(XServerObject server, XInterface object) {
                synchronized (lock) {
                    unreachable = new WaitUnreachable(object);
                }
                server.call(object);
            }
        }

        private final Object lock = new Object();
        private WaitUnreachable unreachable = null;
    }

    private static final class Provider implements XInstanceProvider {
        public Object getInstance(String instanceName) {
            return new XTest() {
                    public void start(XClientObject client) {
                        client.call(
                            new XServerObject() {
                                public void call(XInterface object) {}
                            },
                            new XInterface() {});
                    }
                };
        }
    }

    public interface XClientObject extends XInterface {
        void call(XServerObject server, XInterface object);

        TypeInfo[] UNOTYPEINFO = { new MethodTypeInfo("call", 0, 0) };
    }

    public interface XServerObject extends XInterface {
        void call(XInterface object);

        TypeInfo[] UNOTYPEINFO = { new MethodTypeInfo("call", 0, 0) };
    }

    public interface XTest extends XInterface {
        void start(XClientObject client);

        TypeInfo[] UNOTYPEINFO = { new MethodTypeInfo("start", 0, 0) };
    }
}
