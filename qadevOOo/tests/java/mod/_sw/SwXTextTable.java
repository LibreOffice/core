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
package mod._sw;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;
import util.ValueComparer;

import com.sun.star.beans.XPropertySet;
import com.sun.star.table.XCell;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextRange;
import com.sun.star.text.XTextTable;
import com.sun.star.uno.UnoRuntime;


/**
 * this class tests the implementation of listed interfaces
 * @see com.sun.star.beans.XPropertySet
 * @see com.sun.star.container.XNamed
 * @see com.sun.star.lang.XComponent
 * @see com.sun.star.text.TextContent
 * @see com.sun.star.text.XTextContent
 * @see com.sun.star.text.XTextTable
 */
public class SwXTextTable extends TestCase {
    SOfficeFactory SOF;
    XTextDocument xTextDoc;

    @Override
    protected void initialize(TestParameters tParam, PrintWriter log) throws Exception {
        SOF = SOfficeFactory.getFactory( tParam.getMSF());

        log.println("creating a textdocument");
        xTextDoc = SOF.createTextDoc(null);
    }

    @Override
    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xTextDoc ");
        util.DesktopTools.closeDoc(xTextDoc);
    }

    /**
     this method creates the testenvironment
     @param tParam    TestParameters class which contains additional
        test parameters
     @param log        PrintWriter class to log the test state and result

     @return    TestEnvironment class

     @see TestParameters
     @see PrintWriter
     */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters tParam,
                                                                 PrintWriter log) {
        // creation of testobject here
        XTextTable oObj = null;
        TestEnvironment tEnv = null;
        Object instance = null;

        oObj = SOfficeFactory.createTextTable(xTextDoc);
        instance = SOfficeFactory.createTextTable(xTextDoc);

        int nRow = 6;
        int nCol = 2;
        oObj.initialize(nRow, nCol);

        log.println("Creating instance...");
        tEnv = new TestEnvironment(oObj);

        if (SOfficeFactory.getTableCollection(xTextDoc).getCount() == 0) {
            SOfficeFactory.insertTextContent(xTextDoc, oObj);
        }

        XPropertySet props = UnoRuntime.queryInterface(
                                     XPropertySet.class, oObj);

        try {
            props.setPropertyValue("ChartRowAsLabel", Boolean.TRUE);
            props.setPropertyValue("ChartColumnAsLabel", Boolean.TRUE);
        } catch (Exception e) {
            e.printStackTrace(log);
        }


        //Adding relation for util.XTextTable
        tEnv.addObjRelation("NROW", Integer.valueOf(nRow));
        tEnv.addObjRelation("NCOL", Integer.valueOf(nCol));

        //Adding ObjRelation for XCellRangeData
        Object[][] newData = new Object[6][];
        Object[] line = new Object[2];
        line[0] = new Double(1.5);
        line[1] = new Double(2.5);
        newData[0] = line;
        newData[1] = line;
        newData[2] = line;
        newData[3] = line;
        newData[4] = line;
        newData[5] = line;
        tEnv.addObjRelation("NewData", newData);

        //Adding relation for util.XSortable
        final PrintWriter finalLog = log;
        final XTextTable oTable = oObj;
        tEnv.addObjRelation("SORTCHECKER",
                            new ifc.util._XSortable.XSortChecker() {
            PrintWriter out = finalLog;

            public void setPrintWriter(PrintWriter log) {
                out = log;
            }

            public void prepareToSort() {
                oTable.getCellByName("A1").setValue(4);

                XTextRange textRange = UnoRuntime.queryInterface(
                                               XTextRange.class,
                                               oTable.getCellByName("A2"));
                textRange.setString("b");
                oTable.getCellByName("A3").setValue(3);
                textRange = UnoRuntime.queryInterface(
                                    XTextRange.class,
                                    oTable.getCellByName("A4"));
                textRange.setString("a");
                oTable.getCellByName("A5").setValue(23);
                textRange = UnoRuntime.queryInterface(
                                    XTextRange.class,
                                    oTable.getCellByName("A6"));
                textRange.setString("ab");

            }

            public boolean checkSort(boolean isSortNumbering,
                                     boolean isSortAscending) {
                out.println("Sort checking...");

                boolean res = false;
                String[] value = new String[6];

                for (int i = 0; i < 6; i++) {
                    XCell cell = oTable.getCellByName("A" + (i + 1));
                    XTextRange textRange = UnoRuntime.queryInterface(
                                                   XTextRange.class, cell);
                    value[i] = textRange.getString();
                }

                if (isSortNumbering) {
                    if (isSortAscending) {
                        String[] rightVal = { "3", "4", "23" };
                        String[] vals = { value[3], value[4], value[5] };
                        res = ValueComparer.equalValue(vals, rightVal);

                        if (!res) {
                            out.println("Expected: 3, 4, 23");
                            out.println("Getting: " + value[3] + ", " +
                                            value[4] + ", " + value[5]);
                        }
                    } else {
                        String[] rightVal = { "23", "4", "3" };
                        String[] vals = { value[0], value[1], value[2] };
                        res = ValueComparer.equalValue(vals, rightVal);

                        if (!res) {
                            out.println("Expected: 23, 4, 3");
                            out.println("Getting: " + value[1] + ", " +
                                            value[2] + ", " + value[3]);
                        }
                    }
                } else {
                    if (isSortAscending) {
                        String[] rightVal = { "23", "3", "4", "a", "ab", "b" };
                        res = ValueComparer.equalValue(value, rightVal);

                        if (!res) {
                            out.println("Expected: 23, 3, 4, a, ab, b");
                            out.println("Getting: " + value[0] + ", " +
                                            value[1] + ", " + value[2] +
                                            ", " + value[3] + ", " +
                                            value[4] + ", " + value[5]);
                        }
                    } else {
                        String[] rightVal = { "b", "ab", "a", "4", "3", "23" };
                        res = ValueComparer.equalValue(value, rightVal);

                        if (!res) {
                            out.println("Expected: b, ab, a, 4, 3, 23");
                            out.println("Getting: " + value[0] + ", " +
                                            value[1] + ", " + value[2] +
                                            ", " + value[3] + ", " +
                                            value[4] + ", " + value[5]);
                        }
                    }
                }

                if (res) {
                    out.println("Sorted correctly");
                } else {
                    out.println("Sorted incorrectly");
                }

                return res;
            }
        });

        tEnv.addObjRelation("CONTENT",
                            UnoRuntime.queryInterface(
                                    XTextContent.class, instance));
        tEnv.addObjRelation("RANGE", xTextDoc.getText().createTextCursor());

        return tEnv;
    }
} // finish class SwXTextTable
