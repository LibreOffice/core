/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XTypeDescriptionEnumerationAccess.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:22:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
