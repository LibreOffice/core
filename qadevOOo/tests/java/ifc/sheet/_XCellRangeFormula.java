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

import com.sun.star.sheet.XCellRangeFormula;

public class _XCellRangeFormula extends lib.MultiMethodTest {

    public XCellRangeFormula oObj = null;

    public String[][] orgValue = null;

    public void _getFormulaArray() {
        Object noArray = tEnv.getObjRelation("noArray");
        if (noArray != null) {
            log.println("Component " + noArray.toString() +
                " doesn't really support this Interface");
            log.println("It doesn't make sense to get an FormulaArray from"
                + " the whole sheet");
            tRes.tested("getFormulaArray()", true);
            return;
        }
        orgValue = oObj.getFormulaArray();
        String[][] newValue = oObj.getFormulaArray();
        newValue[0][0] = "inserted";
        oObj.setFormulaArray(newValue);
        boolean res = oObj.getFormulaArray()[0][0].equals(newValue[0][0]);
        tRes.tested("getFormulaArray()",res);
    }

    public void _setFormulaArray() {
        requiredMethod("getFormulaArray()");
        Object noArray = tEnv.getObjRelation("noArray");
        if (noArray != null) {
            log.println("Component " + noArray.toString() +
                " doesn't really support this Interface");
            log.println("It doesn't make sense to set an FormulaArray over"
                + " the whole sheet");
            tRes.tested("setFormulaArray()", true);
            return;
        }
        oObj.setFormulaArray(orgValue);
        boolean res = oObj.getFormulaArray()[0][0].equals(orgValue[0][0]);
        tRes.tested("setFormulaArray()",res);
    }
}
