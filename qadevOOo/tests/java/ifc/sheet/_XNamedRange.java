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

import java.util.Random;
import java.util.StringTokenizer;

import lib.MultiMethodTest;

import com.sun.star.sheet.NamedRangeFlag;
import com.sun.star.sheet.XNamedRange;
import com.sun.star.table.CellAddress;

/**
* Testing <code>com.sun.star.sheet.XNamedRange</code>
* interface methods :
* <ul>
*  <li><code> getContent()</code></li>
*  <li><code> setContent()</code></li>
*  <li><code> getReferencePosition()</code></li>
*  <li><code> setReferencePosition()</code></li>
*  <li><code> getType()</code></li>
*  <li><code> setType()</code></li>
* </ul> <p>
* After test completion object environment has to be recreated.
* @see com.sun.star.sheet.XNamedRange
*/
public class _XNamedRange extends MultiMethodTest {

    public XNamedRange oObj = null;
    String sContent = "";
    int type = 0;
    CellAddress CA = null;

    /**
    * Test calls the method and compares returned value to value that was set
    * by method <code>setContent()</code>. <p>
    * Has <b> OK </b> status if values are equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setContent() </code> : to have current content </li>
    * </ul>
    */
    public void _getContent() {
        requiredMethod("setContent()");
        String content = oObj.getContent();
        log.println("Returned content is \"" + content + "\"");
        boolean bResult = content.equals(sContent);
        tRes.tested("getContent()", bResult);
    }

    /**
    * Test creates and stores random content and calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns. <p>
    */
    public void _setContent() {
        sContent = getRandomContent("A1;A4:C5;=B2");
        log.println("Set content to \"" + sContent + "\"");
        oObj.setContent(sContent);

        tRes.tested("setContent()", true);
    }

    /**
    * Test calls the method and compares returned value to value that was set
    * by method <code>setType()</code>. <p>
    * Has <b> OK </b> status if values are equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setType() </code> : to have current type </li>
    * </ul>
    */
    public void _getType() {
        requiredMethod("setType()");

        int rtype = oObj.getType();
        log.println("Returned type is " + rtype);

        tRes.tested("getType()", type == rtype);
    }

    /**
    * Test sets random type and stores it. <p>
    * Has <b> OK </b> status if the method successfully returns. <p>
    */
    public void _setType() {
        /*
         * The type must be 0 or a combination of the NamedRangeFlag
         * constants and controls if the named range is listed in
         * dialogs prompting for special ranges
         *
         * NamedRangeFlag:    COLUMN_HEADER
         *                  FILTER_CRITERIA
         *                  PRINT_AREA
         *                  ROW_HEADER
         *
         */
        boolean bResult = true;
        int types[] = { 0,
                        NamedRangeFlag.COLUMN_HEADER,
                        NamedRangeFlag.FILTER_CRITERIA,
                        NamedRangeFlag.PRINT_AREA,
                        NamedRangeFlag.ROW_HEADER
                      };

        Random rnd = new Random();
        type = types[rnd.nextInt(5)];

        oObj.setType(type);
        log.println("The type was set to " + type);

        tRes.tested("setType()", bResult);
    }

    /**
    * Test calls the method and compares returned value to value that was set
    * by method <code>setReferencePosition()</code>. <p>
    * Has <b> OK </b> status if all fields of values are equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setReferencePosition() </code> : to have current reference
    *  position </li>
    * </ul>
    */
    public void _getReferencePosition() {
        requiredMethod("setReferencePosition()");

        CellAddress rCA = oObj.getReferencePosition();
        log.println("getReferencePosition returned (" +
            rCA.Sheet + ", " +
            rCA.Column + ", " + rCA.Row + ")" );

        boolean bResult = rCA.Sheet == CA.Sheet;
        bResult &= rCA.Column == CA.Column;
        bResult &= rCA.Row == CA.Row;

        tRes.tested("getReferencePosition()", bResult);
    }

    /**
    * Test creates and stores cell address and calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns. <p>
    */
    public void _setReferencePosition() {
        CA = new CellAddress((short)0, 2, 3);
        oObj.setReferencePosition(CA);
        log.println("ReferencePosition was set to (" +
            CA.Sheet + ", " +
            CA.Column + ", " + CA.Row + ")");

        tRes.tested("setReferencePosition()", true);
    }


    /**
    * Method make string of random content.
    * @return string of random content
    */
    String getRandomContent(String str) {

        String gRS = "none";
        Random rnd = new Random();

        StringTokenizer ST = new StringTokenizer(str, ";");
        int nr = rnd.nextInt(ST.countTokens());
        if (nr < 1) nr++;

        for (int i = 1; i < nr + 1; i++)
            gRS = ST.nextToken();

        return gRS;

    }

    /**
    * Forces object environment recreation.
    */
    @Override
    protected void after() {
        disposeEnvironment();
    }

}


