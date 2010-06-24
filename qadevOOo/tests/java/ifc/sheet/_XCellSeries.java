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
package ifc.sheet;

import helper.LoggingThread;
import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.sheet.XCellSeries;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.uno.UnoRuntime;
import share.LogWriter;


public class _XCellSeries extends MultiMethodTest {
    public XCellSeries oObj = null;
    protected XSpreadsheet oSheet = null;
    protected boolean isSpreadSheet = false;
    protected boolean fillAuto = true;
    protected boolean forceFillAuto = false;


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

        Boolean myFillAuto = (Boolean) tEnv.getObjRelation("XCELLSERIES_FILLAUTO");

        if (myFillAuto != null) fillAuto = myFillAuto.booleanValue();

        if (tParam.containsKey("force_fillauto")){
            fillAuto = tParam.getBool("force_fillauto");
            forceFillAuto = tParam.getBool("force_fillauto");
        }
    }

    public void _fillAuto() {

        if ((isSpreadSheet && !forceFillAuto) || !fillAuto) {
            log.println("This method consumes to much time for a complete SpreadSheet");
            log.println("Please use parameter '-force_fillauto true' to force this test");
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

            LoggingThread logger = new LoggingThread((LogWriter)log, tParam);
            logger.start();

            log.println(
                    "calling oObj.fillSeries(com.sun.star.sheet.FillDirection.TO_RIGHT, com.sun.star.sheet.FillMode.LINEAR, com.sun.star.sheet.FillDateMode.FILL_DATE_DAY, 2, 8)");
            oObj.fillSeries(com.sun.star.sheet.FillDirection.TO_RIGHT,
                            com.sun.star.sheet.FillMode.LINEAR,
                            com.sun.star.sheet.FillDateMode.FILL_DATE_DAY, 2, 8);
            oSheet.getCellByPosition(0, 4).setFormula("=sum(A1:D1)");

            double getting = oSheet.getCellByPosition(0, 4).getValue();
            boolean locres = (getting == 20);

            logger.finish();

            if (!locres) {
                log.println("Operation failed");
            } else {
                log.println("Successful");
            }

            res &= locres;

            logger = new LoggingThread((LogWriter)log, tParam);
            logger.start();

            log.println(
                    "calling oObj.fillSeries(com.sun.star.sheet.FillDirection.TO_RIGHT, com.sun.star.sheet.FillMode.GROWTH, com.sun.star.sheet.FillDateMode.FILL_DATE_DAY, 2, 16)");
            oObj.fillSeries(com.sun.star.sheet.FillDirection.TO_RIGHT,
                            com.sun.star.sheet.FillMode.GROWTH,
                            com.sun.star.sheet.FillDateMode.FILL_DATE_DAY, 2,
                            16);
            oSheet.getCellByPosition(0, 4).setFormula("=sum(A1:D1)");
            getting = oSheet.getCellByPosition(0, 4).getValue();
            locres = (getting == 30);

            logger.finish();

            if (!locres) {
                log.println("Operation failed");
            } else {
                log.println("Successful");
            }

            res &= locres;

            logger = new LoggingThread((LogWriter)log, tParam);
            logger.start();

            log.println(
                    "calling oObj.fillSeries(com.sun.star.sheet.FillDirection.TO_BOTTOM, com.sun.star.sheet.FillMode.LINEAR, com.sun.star.sheet.FillDateMode.FILL_DATE_DAY, 2, 8)");
            oObj.fillSeries(com.sun.star.sheet.FillDirection.TO_BOTTOM,
                            com.sun.star.sheet.FillMode.LINEAR,
                            com.sun.star.sheet.FillDateMode.FILL_DATE_DAY, 2, 8);
            oSheet.getCellByPosition(4, 0).setFormula("=sum(A1:A4)");
            getting = oSheet.getCellByPosition(4, 0).getValue();
            locres = (getting == 20);

            logger.finish();

            if (!locres) {
                log.println("Operation failed");
            } else {
                log.println("Successful");
            }

            res &= locres;

            logger = new LoggingThread((LogWriter)log, tParam);
            logger.start();

            log.println(
                    "calling oObj.fillSeries(com.sun.star.sheet.FillDirection.TO_BOTTOM, com.sun.star.sheet.FillMode.GROWTH, com.sun.star.sheet.FillDateMode.FILL_DATE_DAY, 2, 16)");
            oObj.fillSeries(com.sun.star.sheet.FillDirection.TO_BOTTOM,
                            com.sun.star.sheet.FillMode.GROWTH,
                            com.sun.star.sheet.FillDateMode.FILL_DATE_DAY, 2,
                            16);
            oSheet.getCellByPosition(4, 0).setFormula("=sum(A1:A4)");
            getting = oSheet.getCellByPosition(4, 0).getValue();
            locres = (getting == 30);

            logger.finish();

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
    public void after() {
        disposeEnvironment();
    }
}