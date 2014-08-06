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
* Test is <b> NOT </b> multithread compliant. <p>
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
    @Override
    public void before() {
        args = (Object[])tEnv.getObjRelation("XInitialization.args");

        if (args == null) throw new StatusException(Status.failed
            ("Relation 'XInitialization.args' not found")) ;
        XInitialization xInit = UnoRuntime.queryInterface(
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
    @Override
    public void after() {
        disposeEnvironment() ;
    }

}

