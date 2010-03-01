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

package ifc.io;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.beans.XPropertySet;
import com.sun.star.io.XObjectInputStream;
import com.sun.star.io.XObjectOutputStream;
import com.sun.star.io.XPersistObject;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.io.XObjectOutputStream</code>
* interface methods:
* <ul>
*   <li><code>writeObject()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'InputStream'</code> (of type <code>XObjectInputStream</code>)</li>
*  persist object for testing of write to stream</ul>
* @see com.sun.star.io.XObjectInputStream
* @see com.sun.star.io.XObjectOutputStream
* @see com.sun.star.io.XPersistObject
*/
public class _XObjectOutputStream extends MultiMethodTest {

    public XObjectOutputStream oObj = null;

    /**
    * Test creates persist object, sets label of object,
    * calls the method for created persist object
    * and checks label of object that was read from input stream. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and labels are equals. <p>
    */
    public void _writeObject() throws Exception {
        XObjectInputStream oInStream = (XObjectInputStream)
            tEnv.getObjRelation("InputStream");
        if (oInStream == null) throw
            new StatusException(Status.failed("Relation 'InputStream' failed"));

        // use own implementation of XPersistObject, so test runs
        // without an office
        XPersistObject objWrite = (XPersistObject)
            tEnv.getObjRelation("PersistObject");
        if (objWrite == null) throw
            new StatusException(Status.failed("Relation 'PersistObject' failed"));

        XPropertySet propObjWrite = (XPropertySet)
            UnoRuntime.queryInterface(XPropertySet.class, objWrite);

        // This XPersistObject has a property called 'String'
        propObjWrite.setPropertyValue("String", "XObjectOutputStream");

        log.println("Writing object with label 'XObjectOutputStream'");
        oObj.writeObject(objWrite);
        XPersistObject readObj = oInStream.readObject();
        XPropertySet propSet = (XPropertySet)
            UnoRuntime.queryInterface(XPropertySet.class, readObj);
        String label = (String)propSet.getPropertyValue("String");
        log.println("Object with label '" + label + "' was read");

        tRes.tested("writeObject()", label.equals("XObjectOutputStream")) ;
    }
}

