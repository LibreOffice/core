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



package ifc.beans;

import com.sun.star.beans.PropertyState;
import com.sun.star.uno.XInterface;
import lib.MultiMethodTest;

import com.sun.star.beans.XPropertyWithState;
import lib.Status;
import lib.StatusException;

/**
* Testing <code>com.sun.star.beans.XPropertyWithState</code>
* interface methods :
* <ul>
*  <li><code> getDefaultAsProperty()</code></li>
*  <li><code> getStateAsProperty()</code></li>
*  <li><code> setToDefaultAsProperty()</code></li>
* </ul> <p>
* @see com.sun.star.beans.XPropertyWithState
*/

public class _XPropertyWithState extends MultiMethodTest {

    /**
     * the test object
     */
    public XPropertyWithState oObj;


    /**
     * Test calls the method.
     * Test has ok status if no
     * <CODE>com.sun.star.lang.WrappedTargetException</CODE>
     * was thrown
     */
    public void _getDefaultAsProperty() {
        try{

            XInterface defaultState = (XInterface) oObj.getDefaultAsProperty();

        } catch (com.sun.star.lang.WrappedTargetException e){
            e.printStackTrace(log);
            throw new StatusException(Status.failed("'com.sun.star.lang.WrappedTargetException' was thrown"));
        }

        tRes.tested("getDefaultAsProperty()", true);
    }

    /**
     * Test is ok if <CODE>getStateAsProperty()</CODE> returns
     * as <CODE>PropertyState</CODE> which is not <CODE>null</CODE>
     */
    public void _getStateAsProperty() {

        boolean res = true;

        PropertyState propState = oObj.getStateAsProperty();

        if (propState == null) {
            log.println("the returned PropertyState is null -> FALSE");
            res = false;
        }

        tRes.tested("getStateAsProperty()", res);
    }

    /**
     * Test calls the method.
     * Test has ok status if no
     * <CODE>com.sun.star.lang.WrappedTargetException</CODE>
     * was thrown
     */
    public void _setToDefaultAsProperty() {
        try{

            oObj.setToDefaultAsProperty();

        } catch (com.sun.star.lang.WrappedTargetException e){
            e.printStackTrace(log);
            throw new StatusException(Status.failed("'com.sun.star.lang.WrappedTargetException' was thrown"));
        }

        tRes.tested("setToDefaultAsProperty()", true);
    }

}
