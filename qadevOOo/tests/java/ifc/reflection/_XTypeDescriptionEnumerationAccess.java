/*************************************************************************
 *
 *  $RCSfile: _XTypeDescriptionEnumerationAccess.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:49:22 $
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
            XTypeDescriptionEnumeration enum =
                        oObj.createTypeDescriptionEnumeration(sString, tClass,
                        TypeDescriptionSearchDepth.INFINITE);
            try {
                log.println("Got an enumeration.");
                while (true) {
                    XTypeDescription desc = enum.nextTypeDescription();
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
