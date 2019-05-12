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



package ifc.util;

import lib.MultiMethodTest;

import com.sun.star.util.XReplaceDescriptor;

/**
 * Testing <code>com.sun.star.util.XReplaceDescriptor</code>
 * interface methods :
 * <ul>
 *  <li><code> getReplaceString()</code></li>
 *  <li><code> setReplaceString()</code></li>
 * </ul> <p>
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.util.XReplaceDescriptor
 */
public class _XReplaceDescriptor extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XReplaceDescriptor oObj = null;

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns not
     * <code>null</code> value.
     */
    public void _getReplaceString(){
        log.println("test for getReplaceString() ");
        tRes.tested("getReplaceString()", oObj.getReplaceString()!=null);
    }

    /**
     * Set a new string and checks the result. <p>
     * Has <b> OK </b> status if the string before setting differs
     * from string after setting. <p>
     */
    public void _setReplaceString(){
        log.println("test for setReplaceString() ");
        String oldReplaceStr = oObj.getReplaceString();
        oObj.setReplaceString("XReplaceDescriptor");
        String cmpReplaceStr = oObj.getReplaceString();
        tRes.tested("setReplaceString()", !(cmpReplaceStr.equals(oldReplaceStr)));
        return;
    }

}  // finish class _XReplaceDescriptor


