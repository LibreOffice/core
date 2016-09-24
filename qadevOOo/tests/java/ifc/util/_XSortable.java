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

import java.io.PrintWriter;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.beans.PropertyValue;
import com.sun.star.table.TableSortField;
import com.sun.star.util.XSortable;


/**
 * Testing <code>com.sun.star.util.XSortable</code>
 * interface methods :
 * <ul>
 *  <li><code> createSortDescriptor()</code></li>
 *  <li><code> sort()</code></li>
 * </ul> <p>
 * This test needs the following object relations :
 * <ul>
 *  <li> <code>'SORTCHECKER'</code> : <code>
*    _XSortable.XSortChecker</code> interface implementation
 *  </li>
 * <ul><p>
 * Test is <b> NOT </b> multithread compliant. <p>
 * @see com.sun.star.util.XSortable
 */
public class _XSortable extends MultiMethodTest {
    // oObj filled by MultiMethodTest
    public XSortable oObj = null;
    XSortChecker checker = null;
    PropertyValue[] oPV = null;

    @Override
    protected void before() {
        checker = (XSortChecker) tEnv.getObjRelation("SORTCHECKER");

        if (checker == null) {
            throw new StatusException(Status.failed(
                                              "Couldn't get relation 'SORTCHECKER'"));
        }

        checker.setPrintWriter(log);
    }

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the length of the returned array
     * is greater than zero. <p>
     */
    public void _createSortDescriptor() {
        boolean bResult = false;

        log.println("test for createSortDescriptor() ");
        oPV = oObj.createSortDescriptor();

        if (oPV.length > 0) {
            bResult = true;

            for (int k = 0; k < oPV.length; k++) {
                log.println("DescriptorProperty " + k + ": Name=" +
                            oPV[k].Name + "; Value=" + oPV[k].Value);

                if (oPV[k].Name.equals("SortFields")) {
                    TableSortField[] tsf = (TableSortField[]) oPV[k].Value;

                    for (int l = 0; l < tsf.length; l++) {
                        log.println("\t isAscending:  " +
                                    tsf[l].IsAscending);
                        log.println("\t IsCaseSensitive:  " +
                                    tsf[l].IsCaseSensitive);
                        log.println("\t CollatorAlgorithm:  " +
                                    tsf[l].CollatorAlgorithm);
                    }
                }
            }
        }

        log.println("Found " + oPV.length + " PropertyValues");
        tRes.tested("createSortDescriptor()", bResult);
    }

    /**
     * Test calls the method using descriptor created before as
     * parameter. <p>
     * Has <b> OK </b> status if the method successfully returns
     * and no exceptions were thrown. <p>
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> createSortDescriptor() </code> : to have a descriptor
     *  for sort. </li>
     * </ul>
     */
    public void _sort() {

        checker.prepareToSort();

        log.println(
                "############## Sort algorithm: Alphanumeric Order: Ascending");
        modifyDescriptor(false, true);
        oObj.sort(oPV);

        boolean res = checker.checkSort(false, true);
        log.println(
                "############################################################");

        log.println(
                "############# Sort algorithm: Alphanumeric Order: Descending");
        modifyDescriptor(false, false);
        oObj.sort(oPV);
        res = checker.checkSort(false, false);
        log.println(
                "############################################################");

        log.println(
                "################# Sort algorithm: Numeric Order: Ascending");
        modifyDescriptor(true, true);
        oObj.sort(oPV);
        res = checker.checkSort(true, true);
        log.println(
                "############################################################");

        log.println(
                "################## Sort algorithm: Numeric Order: Descending");
        modifyDescriptor(true, false);
        oObj.sort(oPV);
        res = checker.checkSort(true, false);
        log.println(
                "############################################################");

        tRes.tested("sort()", res);
    }

    protected void modifyDescriptor(boolean isSortNumeric,
                                    boolean isSortAscending) {
        for (int i = 0; i < oPV.length; i++) {
            if (oPV[i].Name.equals("SortFields")) {
                TableSortField[] TableFields = (TableSortField[]) oPV[i].Value;

                if (TableFields.length == 0) {
                    TableFields = new TableSortField[1];
                    TableFields[0] = new TableSortField();
                }

                for (int k = 0; k < TableFields.length; k++) {
                    TableFields[k].IsAscending = isSortAscending;

                    if (isSortNumeric) {
                        TableFields[k].FieldType = com.sun.star.table.TableSortFieldType.NUMERIC;
                        TableFields[k].CollatorAlgorithm = "numeric";
                    } else {
                        TableFields[k].FieldType = com.sun.star.table.TableSortFieldType.ALPHANUMERIC;
                        TableFields[k].CollatorAlgorithm = "alphanumeric";
                    }
                }

                oPV[i].Value = TableFields;
            }

            if (oPV[i].Name.equals("isSortInTable")) {
                oPV[i].Value = Boolean.TRUE;
            }

            if (oPV[i].Name.equals("IsSortColumns")) {
                oPV[i].Value = Boolean.FALSE;
            }
        }

        log.println("Modified sort descriptor: ");

        if (oPV.length > 0) {
            for (int k = 0; k < oPV.length; k++) {
                log.println("DescriptorProperty " + k + ": Name=" +
                            oPV[k].Name + "; Value=" + oPV[k].Value);

                if (oPV[k].Name.equals("SortFields")) {
                    TableSortField[] tsf = (TableSortField[]) oPV[k].Value;

                    for (int l = 0; l < tsf.length; l++) {
                        log.println("\t isAscending:  " +
                                    tsf[l].IsAscending);
                        log.println("\t IsCaseSensitive:  " +
                                    tsf[l].IsCaseSensitive);
                        log.println("\t CollatorAlgorithm:  " +
                                    tsf[l].CollatorAlgorithm);
                    }
                }
            }
        }
    }

    /**
    * The interface for sort checking.
    */
    public interface XSortChecker {
        void prepareToSort();

        boolean checkSort(boolean isSortNumbering,
                                 boolean isSortAscending);

        void setPrintWriter(PrintWriter log);
    }

    /**
    * Forces environment recreation.
    */
    @Override
    protected void after() {
        disposeEnvironment();
    }

} // finish class _XSortable
