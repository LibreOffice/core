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
import lib.Status;
import lib.StatusException;

import com.sun.star.sheet.XScenarios;
import com.sun.star.table.CellRangeAddress;

/**
* Testing <code>com.sun.star.sheet.XScenarios</code>
* interface methods :
* <ul>
*  <li><code> addNewByName()</code></li>
*  <li><code> removeByName()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'ADDR'</code> (of type <code>CellRangeAddress[]</code>):
*   the array of cell range addresses for adding new scenario </li>
* <ul> <p>
* @see com.sun.star.sheet.XScenarios
*/
public class _XScenarios extends MultiMethodTest {

    public XScenarios oObj = null;

    /**
    * Test adds new scenario, stores name of this scenario and checks that
    * scenario with this name is present in collection. <p>
    * Has <b> OK </b> status if the new scenario is present in collection. <p>
    */
    public void _addNewByName() {
        CellRangeAddress[] oAddr = (CellRangeAddress[])
            tEnv.getObjRelation("ADDR");
        if (oAddr == null) throw new StatusException(Status.failed
            ("Relation 'ADDR' not found"));

        oObj.addNewByName("XScenarios", oAddr, "new");
        tRes.tested("addNewByName()",oObj.hasByName("XScenarios"));
    }

    /**
    * Test removes the scenario with stored name and checks that scenario
    * with this name isn't present in collection. <p>
    * Has <b> OK </b> status if the scenario with stored name isn't present in
    * collection after it was removed. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addNewByName() </code> : to have name of the scenario </li>
    * </ul>
    */
    public void _removeByName() {
        requiredMethod("addNewByName()");
        oObj.removeByName("XScenarios");
        tRes.tested("removeByName()",!oObj.hasByName("XScenarios"));
    }


} // EOC _XScenarios

