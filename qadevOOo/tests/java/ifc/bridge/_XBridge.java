/*************************************************************************
 *
 *  $RCSfile: _XBridge.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:16:23 $
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

