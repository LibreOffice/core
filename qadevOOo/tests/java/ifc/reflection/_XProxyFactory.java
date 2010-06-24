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

package ifc.reflection;

import lib.MultiMethodTest;

import com.sun.star.lang.XInitialization;
import com.sun.star.reflection.XProxyFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XAggregation;

/**
/**
* Testing <code>com.sun.star.reflection.XProxyFactory</code>
* interface methods :
* <ul>
*  <li><code> createProxy()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.reflection.XProxyFactory
*/
public class _XProxyFactory extends MultiMethodTest {
    /** Is initialized in super class(using reflection API)
     * when instantiating the test.
     */
    public XProxyFactory oObj;

    /**
    * First an implementation of
    * <code>com.sun.star.lang.XInitialization</code> interface
    * is made which sets a flag when its <code>initialize()</code>
    * method is called. Then an instance of this implementation
    * is created and a proxy object is created for it. Proxy
    * object is tried to query for <code>XInitialization</code>
    * interface and it's <code>initialize</code> method is
    * called. The goal is to check if the real object method
    * was called throwgh it's proxy. <p>
    * Has <b>OK</b> status if the real object method was
    * called and paramters were passed correctly.
    */
    public void _createProxy() {
        class MyObject implements XInitialization {
            Object[] params;

            public void initialize(Object args[]) {
                params = args;
            }
        }

        MyObject obj = new MyObject();

        XAggregation xAggr = oObj.createProxy(obj);

        XInitialization xInit = (XInitialization)UnoRuntime.queryInterface(
                XInitialization.class, xAggr);

        Object params[] = new Object[0];

        try {
            xInit.initialize(params);
        } catch(com.sun.star.uno.Exception e) {
            log.println("Unexpected exception : " + e.getMessage());
            e.printStackTrace(log);
            tRes.tested("createProxy()", false);
            return;
        }

        tRes.tested("createProxy()",
            util.ValueComparer.equalValue(params, obj.params));
    }
}

