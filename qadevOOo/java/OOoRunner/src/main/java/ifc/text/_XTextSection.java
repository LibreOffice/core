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



package ifc.text;

import lib.MultiMethodTest;

import com.sun.star.text.XTextSection;

/**
 * Testing <code>com.sun.star.text.XTextSection</code>
 * interface methods :
 * <ul>
 *  <li><code> getParentSection()</code></li>
 *  <li><code> getChildSections()</code></li>
 * </ul> <p>
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.text.XTextSection
 */
public class _XTextSection extends MultiMethodTest {

    public static XTextSection oObj = null;
    public XTextSection child = null;

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns non zero legth array.
     */
    public void _getChildSections(){
        boolean bResult = false;
        XTextSection oSect[] = oObj.getChildSections();
        if (oSect.length > 0){
            bResult = true;
            child = oSect[0];
        }
        tRes.tested("getChildSections()",  bResult);
    }

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns not
     * <code>null</code> value.
     */
    public void _getParentSection(){
        requiredMethod("getChildSections()");
        boolean bResult = false;
        bResult = (child.getParentSection() != null);
        tRes.tested("getParentSection()",  bResult);
    }


}  // finish class _XTextSection

