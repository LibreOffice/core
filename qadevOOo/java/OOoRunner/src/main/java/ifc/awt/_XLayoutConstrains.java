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



package ifc.awt;

import lib.MultiMethodTest;

import com.sun.star.awt.Size;
import com.sun.star.awt.XLayoutConstrains;

/**
* Testing <code>com.sun.star.awt.XLayoutConstrains</code>
* interface methods:
* <ul>
*   <li><code> getMinimumSize() </code></li>
*   <li><code> getPreferredSize() </code></li>
*   <li><code> calcAdjustedSize() </code></li>
* </ul><p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.awt.XLayoutConstrains
*/
public class _XLayoutConstrains extends MultiMethodTest {
    public XLayoutConstrains oObj = null;

    /**
    * Test calls the method, then check if returned value is not null.<p>
    * Has <b> OK </b> status if the method returns not null.
    */
    public void _getMinimumSize() {
        Size aSize = oObj.getMinimumSize();
        tRes.tested("getMinimumSize()", aSize != null);
    }

    /**
    * Test calls the method, then check if returned value is not null.<p>
    * Has <b> OK </b> status if the method returns not null.
    */
    public void _getPreferredSize() {
        Size aSize = oObj.getPreferredSize();
        tRes.tested("getPreferredSize()", aSize != null);
    }

    /**
    * Test calls the method with the new size as a parameter.<p>
    * Has <b> OK </b> status if the method returns not null.
    */
    public void _calcAdjustedSize() {
        Size aSize = oObj.calcAdjustedSize(new Size(50,50));
        tRes.tested("calcAdjustedSize()", aSize != null);
    }

}

