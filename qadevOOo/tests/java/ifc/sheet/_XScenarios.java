/*************************************************************************
 *
 *  $RCSfile: _XScenarios.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:02:38 $
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

