/*************************************************************************
 *
 *  $RCSfile: _XCellSeries.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:59:32 $
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
package ifc.sheet;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.sheet.XCellSeries;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.uno.UnoRuntime;


public class _XCellSeries extends MultiMethodTest {
    public XCellSeries oObj = null;
    protected XSpreadsheet oSheet = null;
    boolean isSpreadSheet = false;

    protected void before() {
        oSheet = (XSpreadsheet) tEnv.getObjRelation("SHEET");

        if (oSheet == null) {
            log.println("Object relation oSheet is missing");
            log.println("Trying to query the needed Interface");
            oSheet = (XSpreadsheet) UnoRuntime.queryInterface(
                             XSpreadsheet.class, tEnv.getTestObject());

            if (oSheet == null) {
                throw new StatusException(Status.failed(
                                                  "Object relation oSheet is missing"));
            } else {
                isSpreadSheet = true;
            }

        }
    }

    public void _fillAuto() {

        if (isSpreadSheet) {
            log.println("This method consumes to much time for a complete SpreadSheet");
            tRes.tested("fillAuto()",Status.skipped(true));
            return;
        }

        boolean res = true;

        try {
            oSheet.getCellByPosition(0, 0).setValue(2);

            log.println(
                    "calling oObj.fillAuto(com.sun.star.sheet.FillDirection.TO_RIGHT, 1)");
            oObj.fillAuto(com.sun.star.sheet.FillDirection.TO_RIGHT, 1);
            oSheet.getCellByPosition(0, 4).setFormula("=sum(A1:D1)");

            double getting = oSheet.getCellByPosition(0, 4).getValue();
            boolean locres = (getting == 14);

            if (!locres) {
                log.println("Operation failed");
            } else {
                log.println("Successful");
            }

            res &= locres;

            log.println(
                    "calling oObj.fillAuto(com.sun.star.sheet.FillDirection.TO_BOTTOM, 1)");
            oObj.fillAuto(com.sun.star.sheet.FillDirection.TO_BOTTOM, 1);
            oSheet.getCellByPosition(4, 0).setFormula("=sum(A1:A4)");
            getting = oSheet.getCellByPosition(4, 0).getValue();
            locres = (getting == 14);

            if (!locres) {
                log.println("Operation failed");
            } else {
                log.println("Successful");
            }

            res &= locres;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Couldn't set initial version to cell");
            res = false;
        }

        tRes.tested("fillAuto()", res);
    }

    public void _fillSeries() {

        if (isSpreadSheet) {
            log.println("This method consumes to much time for a complete SpreadSheet");
            tRes.tested("fillSeries()",Status.skipped(true));
            return;
        }

        boolean res = true;

        try {
            oSheet.getCellByPosition(0, 0).setValue(2);

            log.println(
                    "calling oObj.fillSeries(com.sun.star.sheet.FillDirection.TO_RIGHT, com.sun.star.sheet.FillMode.LINEAR, com.sun.star.sheet.FillDateMode.FILL_DATE_DAY, 2, 8)");
            oObj.fillSeries(com.sun.star.sheet.FillDirection.TO_RIGHT,
                            com.sun.star.sheet.FillMode.LINEAR,
                            com.sun.star.sheet.FillDateMode.FILL_DATE_DAY, 2, 8);
            oSheet.getCellByPosition(0, 4).setFormula("=sum(A1:D1)");

            double getting = oSheet.getCellByPosition(0, 4).getValue();
            boolean locres = (getting == 20);

            if (!locres) {
                log.println("Operation failed");
            } else {
                log.println("Successful");
            }

            res &= locres;

            log.println(
                    "calling oObj.fillSeries(com.sun.star.sheet.FillDirection.TO_RIGHT, com.sun.star.sheet.FillMode.GROWTH, com.sun.star.sheet.FillDateMode.FILL_DATE_DAY, 2, 16)");
            oObj.fillSeries(com.sun.star.sheet.FillDirection.TO_RIGHT,
                            com.sun.star.sheet.FillMode.GROWTH,
                            com.sun.star.sheet.FillDateMode.FILL_DATE_DAY, 2,
                            16);
            oSheet.getCellByPosition(0, 4).setFormula("=sum(A1:D1)");
            getting = oSheet.getCellByPosition(0, 4).getValue();
            locres = (getting == 30);

            if (!locres) {
                log.println("Operation failed");
            } else {
                log.println("Successful");
            }

            res &= locres;

            log.println(
                    "calling oObj.fillSeries(com.sun.star.sheet.FillDirection.TO_BOTTOM, com.sun.star.sheet.FillMode.LINEAR, com.sun.star.sheet.FillDateMode.FILL_DATE_DAY, 2, 8)");
            oObj.fillSeries(com.sun.star.sheet.FillDirection.TO_BOTTOM,
                            com.sun.star.sheet.FillMode.LINEAR,
                            com.sun.star.sheet.FillDateMode.FILL_DATE_DAY, 2, 8);
            oSheet.getCellByPosition(4, 0).setFormula("=sum(A1:A4)");
            getting = oSheet.getCellByPosition(4, 0).getValue();
            locres = (getting == 20);

            if (!locres) {
                log.println("Operation failed");
            } else {
                log.println("Successful");
            }

            res &= locres;

            log.println(
                    "calling oObj.fillSeries(com.sun.star.sheet.FillDirection.TO_BOTTOM, com.sun.star.sheet.FillMode.GROWTH, com.sun.star.sheet.FillDateMode.FILL_DATE_DAY, 2, 16)");
            oObj.fillSeries(com.sun.star.sheet.FillDirection.TO_BOTTOM,
                            com.sun.star.sheet.FillMode.GROWTH,
                            com.sun.star.sheet.FillDateMode.FILL_DATE_DAY, 2,
                            16);
            oSheet.getCellByPosition(4, 0).setFormula("=sum(A1:A4)");
            getting = oSheet.getCellByPosition(4, 0).getValue();
            locres = (getting == 30);

            if (!locres) {
                log.println("Operation failed");
            } else {
                log.println("Successful");
            }

            res &= locres;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Couldn't set initial version to cell");
            res = false;
        }

        tRes.tested("fillSeries()", res);
    }

    /**
    * Forces environment recreation.
    */
    protected void after() {
        disposeEnvironment();
    }
}