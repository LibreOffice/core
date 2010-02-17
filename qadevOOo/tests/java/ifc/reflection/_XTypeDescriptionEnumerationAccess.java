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
