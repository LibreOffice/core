/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
