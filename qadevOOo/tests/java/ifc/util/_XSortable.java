/*************************************************************************
 *
 *  $RCSfile: _XSortable.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:32:03 $
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
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.util.XSortable
 */
public class _XSortable extends MultiMethodTest {
    // oObj filled by MultiMethodTest
    public XSortable oObj = null;
    XSortChecker checker = null;
    PropertyValue[] oPV = null;

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
                oPV[i].Value = new Boolean(true);
            }

            if (oPV[i].Name.equals("IsSortColumns")) {
                oPV[i].Value = new Boolean(false);
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
    public static interface XSortChecker {
        public void prepareToSort();

        public boolean checkSort(boolean isSortNumbering,
                                 boolean isSortAscending);

        public void setPrintWriter(PrintWriter log);
    }

    /**
    * Forces environment recreation.
    */
    protected void after() {
        disposeEnvironment();
    }

} // finish class _XSortable
