/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
    @Override
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

