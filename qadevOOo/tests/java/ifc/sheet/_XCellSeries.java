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


    @Override
    protected void before() {
        oSheet = (XSpreadsheet) tEnv.getObjRelation("SHEET");

        if (oSheet == null) {
            log.println("Object relation oSheet is missing");
            log.println("Trying to query the needed Interface");
            oSheet = UnoRuntime.queryInterface(
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
    @Override
    public void after() {
        disposeEnvironment();
    }
}