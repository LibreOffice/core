/*************************************************************************
 *
 *  $RCSfile: _XConnectorShape.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:29:56 $
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


