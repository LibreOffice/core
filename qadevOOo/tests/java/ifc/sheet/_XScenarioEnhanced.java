/*************************************************************************
 *
 *  $RCSfile: _XScenarioEnhanced.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2004-03-19 15:58:28 $
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

import com.sun.star.sheet.XScenarioEnhanced;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.uno.UnoRuntime;

import lib.MultiMethodTest;


/**
 *
 * @author  sw93809
 */
public class _XScenarioEnhanced extends MultiMethodTest {
    public XScenarioEnhanced oObj = null;

    public void before() {
        oObj = (XScenarioEnhanced) UnoRuntime.queryInterface(
                       XScenarioEnhanced.class,
                       tEnv.getObjRelation("ScenarioSheet"));
    }

    public void _getRanges() {
        boolean res = true;
        CellRangeAddress[] getting = oObj.getRanges();
        System.out.println("Count " + getting.length);

        CellRangeAddress first = getting[0];

        if (!(first.Sheet == 1)) {
            log.println(
                    "wrong RangeAddress is returned, expected Sheet=0 and got " +
                    first.Sheet);
            res = false;
        }

        if (!(first.StartColumn == 0)) {
            log.println(
                    "wrong RangeAddress is returned, expected StartColumn=0 and got " +
                    first.StartColumn);
            res = false;
        }

        if (!(first.EndColumn == 10)) {
            log.println(
                    "wrong RangeAddress is returned, expected EndColumn=10 and got " +
                    first.EndColumn);
            res = false;
        }

        if (!(first.StartRow == 0)) {
            log.println(
                    "wrong RangeAddress is returned, expected StartRow=0 and got " +
                    first.StartRow);
            res = false;
        }

        if (!(first.EndRow == 10)) {
            log.println(
                    "wrong RangeAddress is returned, expected EndRow=10 and got " +
                    first.EndRow);
            res = false;
        }

        tRes.tested("getRanges()", res);
    }
}