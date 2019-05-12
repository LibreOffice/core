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
import lib.Status;
import lib.StatusException;

import com.sun.star.beans.XPropertySet;
import com.sun.star.util.XIndent;

/**
* Testing <code>com.sun.star.util.XCancellable</code>
* interface methods :
* <ul>
*  <li><code> decrementIndent()</code></li>
*  <li><code> incrementIndent()</code></li>
* </ul> <p>
* @see com.sun.star.util.XIndent
*/
public class _XIndent extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XIndent oObj = null ;

    protected XPropertySet PropSet = null;

    /**
     * Ensures that the ObjRelation PropSet is given.
     */
    public void before() {
        PropSet = (XPropertySet) tEnv.getObjRelation("PropSet");
        if (PropSet == null) {
            throw new StatusException(Status.failed("No PropertySet given"));
        }
    }

    /**
     * Calls the method. <p>
     * Has <b>OK</b> status if the property 'ParaIndent' is incremented afterwards<p>
     */
    public void _incrementIndent() {
        int oldValue = getIndent();
        oObj.incrementIndent();
        int newValue = getIndent();
        tRes.tested("incrementIndent()", oldValue < newValue) ;
    }

    /**
     * Calls the method. <p>
     * Has <b>OK</b> status if the property 'ParaIndent' is decremented afterwards<p>
     * requires 'incrementIndent()' to be executed first.
     */
    public void _decrementIndent() {
        requiredMethod("incrementIndent()");
        int oldValue = getIndent();
        oObj.decrementIndent();
        int newValue = getIndent();
        tRes.tested("decrementIndent()", oldValue > newValue) ;
    }

    public short getIndent() {
        short ret = 0;
        try {
            ret = ((Short) PropSet.getPropertyValue("ParaIndent")).shortValue();
        } catch (com.sun.star.beans.UnknownPropertyException upe) {
        } catch (com.sun.star.lang.WrappedTargetException wte) {
        }
        return ret;
    }

}  // finish class _XCancellable

