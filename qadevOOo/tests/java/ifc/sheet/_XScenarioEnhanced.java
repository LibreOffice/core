/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XScenarioEnhanced.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:51:33 $
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