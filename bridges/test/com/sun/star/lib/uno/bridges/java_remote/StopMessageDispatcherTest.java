/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StopMessageDispatcherTest.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 13:09:40 $
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
import com.sun.star.lang.DisposedException;
import com.sun.star.lib.TestBed;
import com.sun.star.lib.uno.typeinfo.MethodTypeInfo;
import com.sun.star.lib.uno.typeinfo.TypeInfo;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.XInterface;
import complexlib.ComplexTestCase;

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
            XTest test = (XTest) UnoRuntime.queryInterface(
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
