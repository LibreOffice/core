/*************************************************************************
 *
 *  $RCSfile: _XImportable.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-11-18 16:25:28 $
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
package ifc.util;

import lib.MultiMethodTest;

import com.sun.star.beans.PropertyValue;
import com.sun.star.table.XCellRange;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XImportable;


/**
 * checks the Interface XImportable
 */
public class _XImportable extends MultiMethodTest {
    public XImportable oObj;
    protected PropertyValue[] descriptor = null;
    protected String[] names = new String[] {
        "DatabaseName", "SourceType", "SourceObject", "Native"
    };
    protected Type[] types = new Type[] {
        new Type("string"), new Type("com.sun.star.sheet.DataImportMode"),
        new Type("string"), new Type("boolean")
    };

    /**
     * creates an ImportDescriptor, the gained PropertyValues can be found
     * in com.sun.star.sheet.DatabaseImportDescriptor.<br>
     * Returns OK state is all propertynames and types are the specified.
     */

    public void _createImportDescriptor() {
        boolean res = true;
        boolean locResult = false;

        descriptor = oObj.createImportDescriptor(true);
        log.print("Getting when calling createImportDescriptor(true) --");


        //printPropertyValue(descriptor);
        log.println("done");

        log.print("Checking PropertyNames -- ");
        locResult = checkPropertyNames(descriptor, names);
        log.println("Worked: " + locResult);
        res &= locResult;

        log.print("Checking PropertyTypes -- ");
        locResult = checkPropertyTypes(descriptor, types);
        log.println("Worked: " + locResult);
        res &= locResult;

        descriptor = oObj.createImportDescriptor(false);
        log.print("Getting when calling createImportDescriptor(false) -- ");


        //printPropertyValue(descriptor);
        log.println("done");

        log.print("Checking PropertyNames -- ");
        locResult = checkPropertyNames(descriptor, names);
        log.println("Worked: " + locResult);
        res &= locResult;

        log.print("Checking PropertyTypes -- ");
        locResult = checkPropertyTypes(descriptor, types);
        log.println("Worked - " + locResult);
        res &= locResult;

        tRes.tested("createImportDescriptor()", res);
    }

    public void _doImport() {
        requiredMethod("createImportDescriptor()");
        boolean res = true;

        log.print("Setting the ImportDescriptor (Bibliograpy, Table, biblio) -- ");
        descriptor[0].Value = "Bibliography";
        descriptor[1].Value = com.sun.star.sheet.DataImportMode.TABLE;
        descriptor[2].Value = "biblio";
        log.println("done");

        log.print("Importing data (Bibliograpy, Table, biblio) -- ");
        oObj.doImport(descriptor);
        log.println("done");

        log.println("Checking data");
        res &= checkA1("Identifier");

        log.print("Setting the ImportDescriptor (Bibliograpy, SQL, select Author from biblio) -- ");
        descriptor[0].Value = "Bibliography";
        descriptor[1].Value = com.sun.star.sheet.DataImportMode.SQL;
        descriptor[2].Value = "select Author from biblio";
        log.println("done");

        log.print("Importing data (Bibliograpy, SQL, select Author from biblio) -- ");
        oObj.doImport(descriptor);
        log.println("done");

        log.println("Checking data");
        res &= checkA1("Author");

        tRes.tested("doImport()",res);
    }

    protected void printPropertyValue(PropertyValue[] props) {
        for (int i = 0; i < props.length; i++) {
            log.println("\tName: " + props[i].Name);
            log.println("\tValue: " + props[i].Value);
        }
    }

    protected boolean checkPropertyNames(PropertyValue[] props, String[] names) {
        boolean res = true;

        for (int i = 0; i < props.length; i++) {
            boolean locResult = props[i].Name.equals(names[i]);

            if (!locResult) {
                log.println("PropertyName differs for index " + i);
                log.println("\tGetting: " + props[i].Name);
                log.println("\tExpected: " + props[i].Value);
            }

            res &= locResult;
        }

        return res;
    }

    protected boolean checkPropertyTypes(PropertyValue[] props, Type[] types) {
        boolean res = true;

        for (int i = 0; i < props.length; i++) {
            Type ValueType = new Type(props[i].Value.getClass());
            boolean locResult = ValueType.equals(types[i]);

            if (!locResult) {
                log.println("PropertyType differs for " + props[i].Name);
                log.println("\tGetting: " + ValueType.getTypeName());
                log.println("\tExpected: " + types[i].getTypeName());
            }

            res &= locResult;
        }

        return res;
    }

    protected boolean checkA1(String expected) {
        XCellRange range = (XCellRange) UnoRuntime.queryInterface(XCellRange.class, tEnv.getTestObject());
        boolean res = false;
        try{
            String a1 = range.getCellByPosition(0,0).getFormula();
            res = a1.equals(expected);
            if (!res) {
                log.println("\tResult differs from expectation");
                log.println("\tGetting: "+a1);
                log.println("\tExpected: "+expected);
            } else {
                log.println("successful");
            }
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Couldn't get Cell to check");
        }
        return res;
    }

    /**
     * Dispose environment.
     */
    protected void after() {
        disposeEnvironment();
    }

}