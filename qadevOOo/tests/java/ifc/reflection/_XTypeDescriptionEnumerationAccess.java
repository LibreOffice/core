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


package ifc.reflection;

import lib.MultiMethodTest;

import com.sun.star.reflection.TypeDescriptionSearchDepth;
import com.sun.star.reflection.XTypeDescription;
import com.sun.star.reflection.XTypeDescriptionEnumeration;
import com.sun.star.reflection.XTypeDescriptionEnumerationAccess;
import com.sun.star.uno.TypeClass;

/**
 * Testing <code>com.sun.star.reflection.XTypeDescriptionEnumerationAccess
 * </code><br>
 * Needed object relation:
 * <ul>
 * <li><code>SearchString</code>
 * A string to search for as a type description</li>
 * </ul>
 */
public class _XTypeDescriptionEnumerationAccess extends MultiMethodTest {
    public XTypeDescriptionEnumerationAccess oObj = null;

    /**
     * Search the type database for all information regarding the object
     * relation 'SearchString'. Search depth is infinite and information
     * about all types is gathered.
     */
    public void _createTypeDescriptionEnumeration() {
        int i=0;
        TypeClass[] tClass = new TypeClass[0];
        String sString = (String)tEnv.getObjRelation("SearchString");
        if (sString == null || sString.equals("")) {
            System.out.println("Cannot get object relation 'SearchString'");
            tRes.tested("createTypeDescriptionEnumeration()", false);
            return;
        }
        try {
            XTypeDescriptionEnumeration oEnum =
                        oObj.createTypeDescriptionEnumeration(sString, tClass,
                        TypeDescriptionSearchDepth.INFINITE);
            try {
                log.println("Got an enumeration.");
                while (true) {
                    XTypeDescription desc = oEnum.nextTypeDescription();
                    i++;
                    log.println("\tdesc name: " + desc.getName());
                }
            }
            catch(com.sun.star.container.NoSuchElementException e) {
                log.println(
                        "Correct exception caught for exiting enumeration.");
                log.println("Returned were " + i + " type descriptions.");
            }
            catch(Exception e) {
                log.println("Exception while accessing the enumeration.");
                log.println("Index is " + i);
                log.println(e.getMessage());
                tRes.tested("createTypeDescriptionEnumeration()", false);
                return;
            }
            tRes.tested("createTypeDescriptionEnumeration()", i>0);
        }
        catch(Exception e) {
            log.println("Cannot execute method.");
            log.println(e.getMessage());
            tRes.tested("createTypeDescriptionEnumeration()", false);
        }
    }
}
