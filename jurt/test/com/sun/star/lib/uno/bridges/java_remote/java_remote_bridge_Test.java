/*************************************************************************
 *
 *  $RCSfile: java_remote_bridge_Test.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kr $ $Date: 2000-11-23 15:38:39 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package com.sun.star.lib.uno.bridges.java_remote;


import java.io.IOException;
import java.io.PipedInputStream;
import java.io.PipedOutputStream;

import java.lang.reflect.InvocationTargetException;

import java.util.Vector;


import com.sun.star.bridge.XInstanceProvider;

import com.sun.star.comp.connections.PipedConnection;

import com.sun.star.connection.XConnection;

import com.sun.star.uno.IEnvironment;
import com.sun.star.uno.XInterface;

import com.sun.star.lib.uno.environments.java.java_environment;
import com.sun.star.lib.uno.environments.java.Proxy;

import com.sun.star.lib.uno.environments.remote.IProtocol;


public class java_remote_bridge_Test {
    static IEnvironment __java_environment_A;
    static IEnvironment __java_environment_B;

    static java_remote_bridge __java_remote_bridge_A;
    static java_remote_bridge __java_remote_bridge_B;

    static PipedConnection __xConnection_A;
    static PipedConnection __xConnection_B;

    static XInstanceProvider __xInstanceProvider;

    static byte __bytes[] = new byte[] {(byte)255, (byte)0, (byte)128};

    static boolean test_lifecycle(int objects) throws Exception {
        System.err.println("\tjava_remote_bridge - testing lifecycle...");

        boolean passed = true;

//          XInterface theProxy = (XInterface)__java_remote_bridge_B.mapInterfaceFrom("testinstance", XInterface.class); // map the instance back from oid world to Bs real world
        XInterface theProxy = (XInterface)Proxy.create(__java_remote_bridge_B, "testinstance", XInterface.class, true, false);


        TestInterface theProxy_TestInterfaces[] = new TestInterface[objects];

        for(int i = 0; i < objects; ++ i) {
            theProxy_TestInterfaces[i] = (TestInterface)com.sun.star.uno.UnoRuntime.queryInterface(TestInterface.class, theProxy);

            theProxy_TestInterfaces[i].function(); // call the function
        }

        System.err.println("\t\tobject method called " + TestInterface_Object.__called + " times, should be " + objects + " - passed?" + (TestInterface_Object.__called == objects));


        System.err.println("\t\tbridge A life count: " + __java_remote_bridge_A.getLifeCount() + " should be " + objects + " - passed?" + (__java_remote_bridge_A.getLifeCount() == objects));
        passed = passed && (__java_remote_bridge_A.getLifeCount() == objects);
        System.err.println("\t\tbridge B life count: " + __java_remote_bridge_B.getLifeCount() + " should be " + objects + " - passed?" + (__java_remote_bridge_B.getLifeCount() == objects));
        passed = passed && (__java_remote_bridge_B.getLifeCount() == objects);
        System.err.println("\t\tproxy count:" + Proxy.getInstances() + " should be " + (objects + 1) + " - passed?" + (Proxy.getInstances() == (objects + 1)));
        passed = passed && (Proxy.getInstances() == (objects + 1));

          theProxy = null;
        theProxy_TestInterfaces = null;

        System.err.println("\t\twaiting for gc to clear all proxies");
        while(Proxy.getInstances() > 0) { // try to force proxy recycling
            System.gc();
            System.runFinalization();

            byte bytes[] = new byte[1024];
        }

          IProtocol protocol_A = __java_remote_bridge_A.getProtocol();
          IProtocol protocol_B = __java_remote_bridge_B.getProtocol();

        System.err.println("\t\twaiting for pending messages to be done");
        while(protocol_B.getRequestsSendCount() > protocol_A.getRequestsReceivedCount()) {// wait while messages are pending
            System.err.println("pending:" + (protocol_B.getRequestsSendCount() - protocol_A.getRequestsReceivedCount()));
            Thread.sleep(100);
        }
        System.err.println("\t\tbridge A life count: " + __java_remote_bridge_A.getLifeCount() + " should be 0" + " - passed?" + (__java_remote_bridge_A.getLifeCount() == 0));
        passed = passed && (__java_remote_bridge_A.getLifeCount() == 0);
        System.err.println("\t\tbridge B life count: " + __java_remote_bridge_B.getLifeCount() + " should be 0" + " - passed?" + (__java_remote_bridge_B.getLifeCount() == 0));
        passed = passed && (__java_remote_bridge_B.getLifeCount() == 0);
        System.err.println("\t\tproxy count:" + Proxy.getInstances() + " should be 0" + " - passed?" + (Proxy.getInstances() == 0));
        passed = passed && (Proxy.getInstances() == 0);


        // both bridges should be disposed now

        return passed;
    }


    static boolean test_releasing_of_outmapped_objects() throws InterruptedException {
        System.err.println("\tjava_remote_bridge - testing release of outmapped objects - not implemented yet");

        return true;
    }

    static public boolean test(Vector notpassed, String protocol) throws Exception {
        System.err.println("java_remote_bridge - testing with protocol: " + protocol + "...");
        __xConnection_A = new PipedConnection(new Object[0]);
        __xConnection_B = new PipedConnection(new Object[]{__xConnection_A});

        __java_environment_A = new java_environment(null);
        __java_environment_B = new java_environment(null);


        class yXInstanceProvider implements XInstanceProvider {
            public Object getInstance(String name) throws com.sun.star.container.NoSuchElementException, com.sun.star.uno.RuntimeException {
//                  System.err.println("\t\tTest_XInstanceProvider.getInstance:" + name);
                return new TestInterface_Object(); // create the instance, which is to be mapped from A to B
            }
        }

          __xInstanceProvider = new yXInstanceProvider();

        __java_remote_bridge_A = new java_remote_bridge(__java_environment_A, null, new Object[]{protocol, __xConnection_A, __xInstanceProvider});
        __java_remote_bridge_B = new java_remote_bridge(__java_environment_B, null, new Object[]{protocol, __xConnection_B, null});
//          __java_remote_bridge_A = new java_remote_bridge(__java_environment_A, null, new Object[]{"iiop", __xConnection_A, __xInstanceProvider});
//          __java_remote_bridge_B = new java_remote_bridge(__java_environment_B, null, new Object[]{"iiop", __xConnection_B, null});

          boolean passed =  test_lifecycle(100);
        passed = passed && test_releasing_of_outmapped_objects();

        System.err.println("java_remote_bridge_Test - " + protocol + " test passed?" + passed);
        if(!passed && notpassed != null)
            notpassed.addElement("java_remote_bridge_Test - " + protocol + " test passed?" + passed);

        return passed;
    }

    static public void main(String args[]) throws Exception {
        if(args.length == 1)
            test(null, args[0]);
        else
            test(null, "iiop");

//          System.exit(0);
    }
}

