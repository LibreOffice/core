/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XScenarios.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:51:49 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

