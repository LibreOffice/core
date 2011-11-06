/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
