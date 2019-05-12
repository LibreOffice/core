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



package ifc.task;

import lib.MultiMethodTest;

import com.sun.star.task.XStatusIndicator;
import com.sun.star.task.XStatusIndicatorFactory;

/**
* Testing <code>com.sun.star.task.XStatusIndicatorFactory</code>
* interface methods :
* <ul>
*  <li><code> createStatusIndicator()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.task.XStatusIndicatorFactory
*/
public class _XStatusIndicatorFactory extends MultiMethodTest {

    public XStatusIndicatorFactory    oObj = null;

    /**
     * A status indicator created. Also some actions performed
     * with it, which are not related to test flow. <p>
     * Has <b> OK </b> status if the method returns not null
     * value. <p>
     */
    public void _createStatusIndicator() {
        boolean bResult = true;

        XStatusIndicator SI1 = oObj.createStatusIndicator();
        XStatusIndicator SI2 = oObj.createStatusIndicator();
        //Start all Indicators
        SI1.start("Status1",100);
        SI2.start("Status2",100);

        //change SI2
        SI2.setText("A new name for the status");
        SI2.setValue(45);
        SI2.reset();
        SI2.end();

        //Now SI1 is active
        SI1.end();

        bResult &= SI1 != null;

        tRes.tested("createStatusIndicator()", bResult);
    }
}


