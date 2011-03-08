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
        "DatabaseName", "SourceType", "SourceObject", "IsNative"
    };
    protected Type[] types = new Type[] {
        new Type(String.class), new Type(com.sun.star.sheet.DataImportMode.class),
        new Type(String.class), new Type(Boolean.class)
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
                log.println("\tExpected: " + names[i]);
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