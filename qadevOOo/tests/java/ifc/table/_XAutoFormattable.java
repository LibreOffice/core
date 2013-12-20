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
package ifc.table;

import java.util.Random;

import lib.MultiMethodTest;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.table.XAutoFormattable;
import com.sun.star.table.XCell;
import com.sun.star.table.XCellRange;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;


/**
* Testing <code>com.sun.star.table.XAutoFormattable</code>
* interface methods :
* <ul>
*  <li><code> autoFormat()</code></li>
* </ul> <p>
* The component tested <b>must implement</b> interface
* <code>com.sun.star.table.XCellRange</code>. <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.table.XAutoFormattable
*/
public class _XAutoFormattable extends MultiMethodTest {
    public XAutoFormattable oObj = null;

    /**
    * First 'Default' autoformat is set and a background of a cell
    * is obtained. Then any other autoformat is set and background
    * of a cell is obtained again.<p>
    * Has <b> OK </b> status if backgrounds with different autoformat
    * settings are differ. <p>
    */
    public void _autoFormat() {
        boolean bResult = true;
        XMultiServiceFactory oMSF = (XMultiServiceFactory) tParam.getMSF();
        String name = "Default";

        try {
            oObj.autoFormat(name); // applying default format

            // getting current background of the cell
            XCellRange cellRange = UnoRuntime.queryInterface(
                                           XCellRange.class, oObj);
            XCell oCell = cellRange.getCellByPosition(0, 0);
            XPropertySet PS = UnoRuntime.queryInterface(
                                      XPropertySet.class, oCell);

            Integer bkgrnd1;
            try {
                bkgrnd1 = (Integer) PS.getPropertyValue("CellBackColor");
            } catch (com.sun.star.beans.UnknownPropertyException e) {
                bkgrnd1 = (Integer) PS.getPropertyValue("BackColor");
            }

            // getting formats names.
            XInterface iFormats = (XInterface) oMSF.createInstance(
                                          "com.sun.star.sheet.TableAutoFormats");
            XNameAccess formats = UnoRuntime.queryInterface(
                                          XNameAccess.class, iFormats);
            String[] names = formats.getElementNames();

            // getting one random not default style name
            Random rnd = new Random();

            if (names.length > 1) {
                while (name.equals("Default")) {
                    name = names[rnd.nextInt(names.length)];
                }
            } else {
                name = names[0];
            }

            log.println("Applying style " + name);


            // applying style
            oObj.autoFormat(name);

            // getting new cell's background.
            Integer bkgrnd2;
            try {
                bkgrnd2 = (Integer) PS.getPropertyValue("CellBackColor");
            } catch (com.sun.star.beans.UnknownPropertyException e) {
                bkgrnd2 = (Integer) PS.getPropertyValue("BackColor");
            }

            bResult &= !bkgrnd1.equals(bkgrnd2);
        } catch (com.sun.star.uno.Exception e) {
            log.println("Exception occurred :");
            e.printStackTrace(log);
            bResult = false;
        }

        tRes.tested("autoFormat()", bResult);
    }

    /**
    * Forces environment recreation.
    */
    protected void after() {
        disposeEnvironment();
    }
}