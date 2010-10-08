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
* Test is <b> NOT </b> multithread compilant. <p>
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
    public void before() {
        log.println("No shapes implementing XConnectableShape still found.");
        XShape[] shapes = (XShape[])
            tEnv.getObjRelation("XConnectorShape.Shapes") ;
        if (shapes == null) throw new StatusException(Status.failed
            ("Relation not found.")) ;
        shape1 = (XConnectableShape) UnoRuntime.queryInterface
            (XConnectableShape.class, shapes[0]) ;
        shape2 = (XConnectableShape) UnoRuntime.queryInterface
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


