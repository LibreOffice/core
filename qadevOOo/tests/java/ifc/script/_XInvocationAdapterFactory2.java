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

package ifc.script;

import lib.MultiMethodTest;
import lib.StatusException;

import com.sun.star.io.XInputStream;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.script.XInvocation;
import com.sun.star.script.XInvocationAdapterFactory2;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Testing <code>com.sun.star.script.XInvocationAdapterFactory</code>
* interface methods :
* <ul>
*  <li><code> createAdapter()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.script.XInvocationAdapterFactory
*/
public class _XInvocationAdapterFactory2 extends MultiMethodTest {

    /**
     * oObj filled by MultiMethodTest
     */
    public XInvocationAdapterFactory2 oObj = null;

    /**
    * First an invocation object of <code>com.sun.star.io.Pipe</code>
    * instance is created using <code>com.sun.star.script.Invocation
    * </code> service. Then trying to create an adapter of this
    * invocation for <code>com.sun.star.io.XInputStream</code>
    * interface. <p>
    * Has <b>OK</b> status if the adapter returned is successfully
    * queried for <code>com.sun.star.io.XInputStream</code>
    * interface.
    * @see com.sun.star.script.Invocation
    * @see com.sun.star.script.XInvocation
    * @see com.sun.star.io.Pipe
    */
    public void _createAdapter() {
        XInvocation xInv = null ;
        XMultiServiceFactory xMSF = null;
        try {
            xMSF = (XMultiServiceFactory)tParam.getMSF();
            Object[] args = {xMSF.createInstance
                ("com.sun.star.io.Pipe")};

            Object oInvFac = xMSF.createInstance
                ("com.sun.star.script.Invocation") ;

            XSingleServiceFactory xInvFac = (XSingleServiceFactory) UnoRuntime.
                queryInterface(XSingleServiceFactory.class, oInvFac) ;

            Object oInv = xInvFac.createInstanceWithArguments(args) ;

            xInv = (XInvocation) UnoRuntime.queryInterface
                (XInvocation.class, oInv) ;

        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log) ;
            throw new StatusException("Cann't create invocation for object", e) ;
        }

        XInterface xInStr = null ;

        Type[] types = new Type[1];
        types[0] = new Type(XInputStream.class);

        Object adp = oObj.createAdapter(xInv,types);

        xInStr = (XInterface) UnoRuntime.queryInterface
                (XInputStream.class, adp) ;


        if (xInStr != null)
            tRes.tested("createAdapter()", true) ;
        else {
            log.println("Adapter created doesn't implement required interface") ;
            tRes.tested("createAdapter()", false) ;
        }
    }
}

