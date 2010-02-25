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

