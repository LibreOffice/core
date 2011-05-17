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
package mod._sw;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;
import util.ValueComparer;

import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.XMultiServiceFactory;
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

    protected void initialize(TestParameters tParam, PrintWriter log) {
        SOF = SOfficeFactory.getFactory( (XMultiServiceFactory) tParam.getMSF());

        try {
            log.println("creating a textdocument");
            xTextDoc = SOF.createTextDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace(log);
            throw new StatusException("Couldn't create document", e);
        }
    }

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
    protected synchronized TestEnvironment createTestEnvironment(TestParameters tParam,
                                                                 PrintWriter log) {
        // creation of testobject here
        XTextTable oObj = null;
        TestEnvironment tEnv = null;
        Object instance = null;

        try {
            oObj = SOF.createTextTable(xTextDoc);
            instance = SOF.createTextTable(xTextDoc);
        } catch (com.sun.star.uno.Exception uE) {
            uE.printStackTrace(log);
            throw new StatusException("Couldn't create testobj : " +
                                      uE.getMessage(), uE);
        }

        int nRow = 6;
        int nCol = 2;
        oObj.initialize(nRow, nCol);

        log.println("Creating instance...");
        tEnv = new TestEnvironment(oObj);

        if (SOF.getTableCollection(xTextDoc).getCount() == 0) {
            try {
                SOF.insertTextContent(xTextDoc, oObj);
            } catch (com.sun.star.uno.Exception uE) {
                uE.printStackTrace(log);
                throw new StatusException("Couldn't create TextTable : " +
                                          uE.getMessage(), uE);
            }
        }

        XPropertySet props = (XPropertySet) UnoRuntime.queryInterface(
                                     XPropertySet.class, oObj);

        try {
            props.setPropertyValue("ChartRowAsLabel", new Boolean(true));
            props.setPropertyValue("ChartColumnAsLabel", new Boolean(true));
        } catch (Exception e) {
            e.printStackTrace(log);
        }


        //Adding relation for util.XTextTable
        tEnv.addObjRelation("NROW", new Integer(nRow));
        tEnv.addObjRelation("NCOL", new Integer(nCol));

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

                XTextRange textRange = (XTextRange) UnoRuntime.queryInterface(
                                               XTextRange.class,
                                               oTable.getCellByName("A2"));
                textRange.setString("b");
                oTable.getCellByName("A3").setValue(3);
                textRange = (XTextRange) UnoRuntime.queryInterface(
                                    XTextRange.class,
                                    oTable.getCellByName("A4"));
                textRange.setString("a");
                oTable.getCellByName("A5").setValue(23);
                textRange = (XTextRange) UnoRuntime.queryInterface(
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
                    XTextRange textRange = (XTextRange) UnoRuntime.queryInterface(
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
                    out.println("Sorted uncorrectly");
                }

                return res;
            }
        });

        tEnv.addObjRelation("CONTENT",
                            (XTextContent) UnoRuntime.queryInterface(
                                    XTextContent.class, instance));
        tEnv.addObjRelation("RANGE", xTextDoc.getText().createTextCursor());

        return tEnv;
    }
} // finish class SwXTextTable
