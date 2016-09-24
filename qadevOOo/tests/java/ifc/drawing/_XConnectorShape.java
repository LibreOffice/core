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

package ifc.drawing;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.drawing.ConnectionType;
import com.sun.star.drawing.XConnectableShape;
import com.sun.star.drawing.XConnectorShape;
import com.sun.star.drawing.XShape;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.drawing.XConnectorShape</code>
* interface methods :
* <ul>
*  <li><code> connectStart()</code></li>
*  <li><code> connectEnd()</code></li>
*  <li><code> disconnectBegin()</code></li>
*  <li><code> disconnectEnd()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'XConnectorShape.Shapes'</code>
*  (of type <code>com.sun.star.drawing.XShape[]</code>):
*   an array of two shapes which <b>must</b> implement
*   <code>com.sun.star.drawing.XConnectableShape</code>
*   interface and are used for being connected by
*   connector shape.</li>
* <ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.drawing.XConnectorShape
*/
public class _XConnectorShape extends MultiMethodTest {

    public XConnectorShape oObj = null;        //oObj filled by MultiMethodTest
    private XConnectableShape shape1 = null,
        shape2 = null ;

    /**
     * Retrieves object relation.
     * @throw StatusException If the relation is not found or shapes don't
     * support <code>XConnectableShape</code> interface.
     */
    @Override
    public void before() {
        log.println("No shapes implementing XConnectableShape still found.");
        XShape[] shapes = (XShape[])
            tEnv.getObjRelation("XConnectorShape.Shapes") ;
        if (shapes == null) throw new StatusException(Status.failed
            ("Relation not found.")) ;
        shape1 = UnoRuntime.queryInterface
            (XConnectableShape.class, shapes[0]) ;
        shape2 = UnoRuntime.queryInterface
            (XConnectableShape.class, shapes[1]) ;
        if (shape1 == null || shape2 == null) throw new StatusException
            (Status.failed("Shapes don't implement XConnectableShape"+
                " interface.")) ;
    }


    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    */
    public void _connectStart() {
        oObj.connectStart(shape1, ConnectionType.AUTO);

        tRes.tested("connectStart()", true) ;
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    */
    public void _connectEnd() {
        oObj.connectEnd(shape2, ConnectionType.AUTO);

        tRes.tested("connectEnd()", true) ;
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> connectStart() </code> : first shape needs to be
    *    connected. </li>
    * </ul>
    */
    public void _disconnectBegin() {
        requiredMethod("connectStart()");

        oObj.disconnectBegin(shape1);

        tRes.tested("disconnectBegin()", true) ;
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> connectEnd() </code> : first shape needs to be
    *    connected. </li>
    * </ul>
    */
    public void _disconnectEnd() {
        requiredMethod("connectEnd()");

        oObj.disconnectEnd(shape2);

        tRes.tested("disconnectEnd()", true) ;
    }
}


