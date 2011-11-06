/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

