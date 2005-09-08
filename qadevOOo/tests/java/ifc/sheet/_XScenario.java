/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XScenario.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:51:15 $
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

import com.sun.star.sheet.XScenario;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.uno.Any;
import com.sun.star.uno.XInterface;
import lib.MultiMethodTest;
import lib.Status;
import util.ValueComparer;

/**
 *
 */
public class _XScenario extends MultiMethodTest {
    public XScenario oObj = null;
    CellRangeAddress address = null;
    String comment = null;
    boolean skipTest = false;

    public void before() {
        // testing a scenario containing the whole sheet does not make sense.
        // test is skipped until this interface is implemented somewhere else
        skipTest = true;
    }

    public void _addRanges() {
        if (skipTest) {
            tRes.tested("addRanges()",Status.skipped(true));
            return;
        }
        oObj.addRanges(new CellRangeAddress[] {address});
        tRes.tested("addRanges()", true);
    }

    public void _apply() {
        requiredMethod("addRanges()");
        if (skipTest) {
            tRes.tested("apply()",Status.skipped(true));
            return;
        }
        oObj.apply();
        tRes.tested("apply()", true);
    }

    public void _getIsScenario() {
        requiredMethod("apply()");
        if (skipTest) {
            tRes.tested("getIsScenario()",Status.skipped(true));
            return;
        }
        boolean getIs = oObj.getIsScenario();
        tRes.tested("getIsScenario()", getIs);
    }

    public void _getScenarioComment() {
        if (skipTest) {
            tRes.tested("getScenarioComment()",Status.skipped(true));
            return;
        }
        comment = oObj.getScenarioComment();
        tRes.tested("getScenarioComment()", true);
    }

    public void _setScenarioComment() {
        requiredMethod("getScenarioComment()");
        if (skipTest) {
            tRes.tested("setScenarioComment()",Status.skipped(true));
            return;
        }
        boolean res = false;
        oObj.setScenarioComment("MyComment");
        String c = oObj.getScenarioComment();
        res = c.equals("MyComment");
        oObj.setScenarioComment(comment);
        tRes.tested("setScenarioComment()", res);
    }
}
