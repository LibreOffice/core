/*************************************************************************
 *
 *  $RCSfile: _XScenario.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Date: 2004-11-02 11:57:22 $
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
