/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: UnoRuntime_EnvironmentTest.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:17:54 $
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

package com.sun.star.uno;

import com.sun.star.comp.connections.PipedConnection;
import complexlib.ComplexTestCase;
import util.WaitUnreachable;

public final class UnoRuntime_EnvironmentTest extends ComplexTestCase {
    public String getTestObjectName() {
        return getClass().getName();
    }

    public String[] getTestMethodNames() {
        return new String[] { "test_getEnvironment", "test_getBridge" };
    }

    public void test_getEnvironment() throws java.lang.Exception {
        Object o1 = new Object();
        Object o2 = new Object();

        // get two environments with different contexts
        WaitUnreachable java_environment1 = new WaitUnreachable(
            UnoRuntime.getEnvironment("java", o1));
        WaitUnreachable java_environment2 = new WaitUnreachable(
            UnoRuntime.getEnvironment("java", o2));

        // ensure that the environments are different
        assure("", java_environment1.get() != java_environment2.get());

        // test if we get the same environment when we reget it
        assure("",
               UnoRuntime.areSame(java_environment1.get(),
                                  UnoRuntime.getEnvironment("java", o1)));
        assure("",
               UnoRuntime.areSame(java_environment2.get(),
                                  UnoRuntime.getEnvironment("java", o2)));

        // drop the environments and wait until they are gc
        java_environment1.waitUnreachable();
        java_environment2.waitUnreachable();
    }

    public void test_getBridge() throws java.lang.Exception {
        PipedConnection oneSide = new PipedConnection(new Object[0]);
        PipedConnection otherSide = new PipedConnection(new Object[]{oneSide});

        // get a bridge
        IBridge iBridge = UnoRuntime.getBridgeByName(
            "java", null, "remote", "testname",
            new Object[] { "urp", oneSide, null });

        // reget the bridge, it must be the same as above
        IBridge iBridge_tmp = UnoRuntime.getBridgeByName(
            "java", null, "remote", "testname",
            new Object[] { "urp", oneSide, null });
        assure("", UnoRuntime.areSame(iBridge_tmp, iBridge));

        // dispose the bridge, this removes the entry from the runtime
        iBridge.dispose();

        // reget the bridge, it must be a different one
        iBridge_tmp = UnoRuntime.getBridgeByName(
            "java", null, "remote", "testname",
            new Object[]{ "urp", oneSide, null });
        assure("", !UnoRuntime.areSame(iBridge_tmp, iBridge));
    }
}
