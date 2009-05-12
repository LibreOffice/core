/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: CallbackClass.java,v $
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

package complex.toolkit;

import com.sun.star.awt.XCallback;
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
public class CallbackClass implements XCallback{

    private LogWriter log;

    private XMultiServiceFactory xMSF;


    public CallbackClass(LogWriter log, XMultiServiceFactory xMSF ) {

        this.xMSF = xMSF;
        this.log = log;
    }


    /**
     * Callback method which will be called by the asynchronous
     * service where we have added our request before.
     */
    public void notify( Object aData ) {

        log.println("callback called successfully" );
    }
}
