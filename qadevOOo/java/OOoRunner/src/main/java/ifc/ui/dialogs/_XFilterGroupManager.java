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



package ifc.ui.dialogs;

import lib.MultiMethodTest;

import com.sun.star.beans.StringPair;
import com.sun.star.ui.dialogs.XFilterGroupManager;

/**
* Testing <code>com.sun.star.ui.dialogs.XFilterGroupManager</code>
* @see com.sun.star.ui.XFilterGroupManager
*/
public class _XFilterGroupManager extends MultiMethodTest {

    public XFilterGroupManager oObj = null;

    /**
    * Appends a new FilterGroup (for extension 'txt'). <p>
    * Has <b>OK</b> status if no runtime exceptions ocured.
    */
    public void _appendFilterGroup() {
        boolean res = true;
        try {
            StringPair[] args = new StringPair[1];
            args[0] = new StringPair();
            args[0].First = "ApiTextFiles";
            args[0].Second = "txt";
            oObj.appendFilterGroup("TestFilter", args);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            res=false;
        }

        tRes.tested("appendFilterGroup()", res) ;
    }

}

