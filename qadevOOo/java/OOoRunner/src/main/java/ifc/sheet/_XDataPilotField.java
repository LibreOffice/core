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

import com.sun.star.container.XIndexAccess;
import com.sun.star.sheet.XDataPilotField;

import lib.MultiMethodTest;


public class _XDataPilotField extends MultiMethodTest {
    public XDataPilotField oObj = null;

    public void _getItems() {
        XIndexAccess xIA = oObj.getItems();
        tRes.tested("getItems()", checkIndexAccess(xIA));
    }

    /**
     * calls the method getCount at the IndexAccess, returns true is it is >0
     * and getByIndex() doesn't throw an exception for Indexes between 0 and count
     */
    protected boolean checkIndexAccess(XIndexAccess xIA) {
        boolean res = true;
        int count = xIA.getCount();
        log.println("Found " + count + " Elements");
        res &= (count > 0);

        for (int k = 0; k < count; k++) {
            try {
                Object element = xIA.getByIndex(k);
                log.println("Element " + k + " = " + element);
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                log.println("Unexpected Exception while getting by Index (" + k +
                            ")" + e.getMessage());
                res &= false;
            } catch (com.sun.star.lang.WrappedTargetException e) {
                log.println("Unexpected Exception while getting by Index (" + k +
                            ")" + e.getMessage());
                res &= false;
            }
        }

        return res;
    }
}