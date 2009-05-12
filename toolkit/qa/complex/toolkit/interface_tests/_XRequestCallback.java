/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: _XRequestCallback.java,v $
 * $Revision: 1.3 $
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

package complex.toolkit.interface_tests;

import com.sun.star.awt.XRequestCallback;
import lib.MultiMethodTest;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;
import share.LogWriter;

/**
 * Testing <code>com.sun.star.awt.XRequestCallback</code>
 * interface methods :
 * <ul>
 *  <li><code> addCallback()</code></li>
 * </ul> <p>
 * @see com.sun.star.awt.XRequestCallback
 */
public class _XRequestCallback {

    private LogWriter log;

    private static final String className =
        "com.sun.star.awt.XRequestCallback" ;

    public XRequestCallback oObj = null;
    private XMultiServiceFactory xMSF;

    String text = null;


    public _XRequestCallback(XInterface object, LogWriter log, XMultiServiceFactory xMSF ) {
        oObj = (XRequestCallback)UnoRuntime.queryInterface(
                                    XRequestCallback.class, object);
        this.xMSF = xMSF;
        this.log = log;
    }


    /**
     * Calls the method and checks returned value.
     * Has OK status if returned value is equal to <code>chCount - 1</code>.
     * The following method tests are to be executed before:
     * <ul>
     *  <li> <code>addCallback()</code> </li>
     * </ul>
     */
    public boolean _addCallback() {

        boolean res = true;
        try {
            Object a = new Object();
            oObj.addCallback( null, a );
        } catch (com.sun.star.uno.RuntimeException ie) {
            res = false;
        }
        log.println("addCallback called" );

        return res;
    }

}
