/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package ifc.bridge;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.bridge.XBridge;
import com.sun.star.connection.XConnection;
import com.sun.star.lang.XInitialization;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;


/**
* Testing <code>com.sun.star.bridge.XBridge</code>
* interface methods :
* <ul>
*  <li><code> getInstance()</code></li>
*  <li><code> getName()</code></li>
*  <li><code> getDescription()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'XInitialization.args'</code> (of type <code>Object[]</code>):
*   relation which contains arguments for Bridge initialization.
*   It used here to check description of the bridge. This array
*   must contain : [0] - the name of the bridge, [1] - the name of
*   protocol, [2] - <code>XConnection</code> reference to bridge
*   connection. </li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* After test completion object environment has to be recreated.
* @see com.sun.star.bridge.XBridge
*/
public class _XBridge extends MultiMethodTest {

    public XBridge oObj;

    protected Object[] args;//for object relation 'XInitialization.args'

    /**
    * Retrieves object relations.
    * @throws StatusException If one of relations not found.
    */
    public void before() {
        args = (Object[])tEnv.getObjRelation("XInitialization.args");

        if (args == null) throw new StatusException(Status.failed
            ("Relation 'XInitialization.args' not found")) ;
        XInitialization xInit = (XInitialization)UnoRuntime.queryInterface(
                                       XInitialization.class, oObj);
        try {
            xInit.initialize(args);
        }
        catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Can't initialize the bridge", e);
        }
    }

    /**
    * Tries to retrieve <code>ServiceManager</code> service
    * using the bridge. <p>
    * Has <b>OK</b> status if non null object returned.
    */
    public void _getInstance() {
        XInterface xInt = (XInterface)oObj.getInstance(
                "com.sun.star.lang.ServiceManager");

        tRes.tested("getInstance()", xInt != null);
    }

    /**
    * Retrieves the name of the bridge from relation and compares
    * it to name returned by the method. <p>
    * Has <b>OK</b> status if names are equal.
    */
    public void _getName() {
        String expectedName = (String)args[0]; // args[0] - bridge name

        String name = oObj.getName();

        if (!tRes.tested("getName()", name.equals(expectedName))) {
            log.println("getName() returns wrong result : \"" + name + "\"");
            log.println("expected = \"" + expectedName + "\"");
        }
    }

    /**
    * Retrieves the description of the bridge and compares it with
    * expected description composed using relation
    * <code> ([protocol] + ":" + [connection description]) </code>. <p>
    * Has <b>OK</b> status if description returned by the method
    * is equal to expected one.
    */
    public void _getDescription() {
        String protocol = (String)args[1]; // args[1] - protocol
        XConnection xConnection = (XConnection)args[2]; // args[2] - connection
        // expected description is protocol + ":" + xConnection.getDescription()
        String expectedDescription =
                protocol + ":" + xConnection.getDescription();

        String description = oObj.getDescription();

        if (!tRes.tested("getDescription()",
                description.equals(expectedDescription))) {
            log.println("getDescription() returns wrong result : \""
                    + description + "\"");
            log.println("expected = \"" + expectedDescription + "\"");
        }
    }

    /**
    * Disposes object environment.
    */
    public void after() {
        disposeEnvironment() ;
    }

}

