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

import lib.MultiMethodTest;

import com.sun.star.sheet.XDatabaseRanges;
import com.sun.star.table.CellRangeAddress;

/**
* Testing <code>com.sun.star.sheet.XDatabaseRanges</code>
* interface methods :
* <ul>
*  <li><code> addNewByName()</code></li>
*  <li><code> removeByName()</code></li>
* </ul> <p>
* @see com.sun.star.sheet.XDatabaseRanges
*/
public class _XDatabaseRanges extends MultiMethodTest {

    public XDatabaseRanges oObj = null;
    CellRangeAddress CRA = null;
    String name = null;

    /**
    * Test adds a new database range to the collection, checks that range with
    * this name exist in collection and then tries to add range with the same
    * name. <p>
    * Has <b> OK </b> status if the added range exists in collection and
    * exception was thrown when trying to add range with name that is same as name
    * of existent range. <p>
    */
    public void _addNewByName() {
        boolean bResult = true;
        log.println("Trying to add range with proper name.");

        CRA = new CellRangeAddress((short)0, 1, 2, 3, 4);
        name = "_XDatabaseRanges_addNewByRange";

        oObj.addNewByName(name, CRA);

        bResult &= oObj.hasByName(name);

        if (bResult) log.println("Ok");
        log.println("Trying to add existing element.");

        try {
            oObj.addNewByName(name, CRA);
            log.println("Exception expected... Test failed.");
            bResult = false;
        } catch(com.sun.star.uno.RuntimeException e) {
            log.println("Exception occurred while testing addNewByName() : " + e);
            bResult = true;
        }

        tRes.tested("addNewByName()", bResult);
    }

    /**
    * Test removes the database range with name that exist exactly and then
    * tries to remove the range with name that doesn't exist exactly. <p>
    * Has <b> OK </b> status if first range was successfully removed and
    * exception was thrown when trying to remove non-existent database range.<p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addNewByName() </code> : to have definitely existed database
    *  range </li>
    * </ul>
    */
    public void _removeByName(){
        boolean bResult = true;
        requiredMethod("addNewByName()");

        log.println("Remove inserted element.");

        try {
            oObj.removeByName(name);
            bResult &= !oObj.hasByName(name);
        } catch (com.sun.star.uno.RuntimeException e) {
            log.println("Exception occurred while testing removeByName() : " + e);
            bResult = false;
        }

        log.println("OK.\nTrying to remove unexistant element.");

        try {
            oObj.removeByName(name);
            log.println("Exception expected... - FAILED");
            bResult = false;
        } catch (com.sun.star.uno.RuntimeException e) {
            log.println("Expected exception. - OK : " + e);
        }
        tRes.tested("removeByName()", bResult);
    }
}

