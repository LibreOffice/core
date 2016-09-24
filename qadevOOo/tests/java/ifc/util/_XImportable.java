/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
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
        XCellRange range = UnoRuntime.queryInterface(XCellRange.class, tEnv.getTestObject());
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
    @Override
    protected void after() {
        disposeEnvironment();
    }

}