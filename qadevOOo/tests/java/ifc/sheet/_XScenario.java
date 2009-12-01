/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: _XScenario.java,v $
 * $Revision: 1.4 $
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

import com.sun.star.sheet.XScenario;
import com.sun.star.table.CellRangeAddress;
import lib.MultiMethodTest;
import lib.Status;

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
