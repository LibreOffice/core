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

package ifc.java;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.java.XJavaThreadRegister_11;

/**
* Testing <code>com.sun.star.java.XJavaThreadRegister_11</code>
* interface methods :
* <ul>
*  <li><code> isThreadAttached()</code></li>
*  <li><code> registerThread()</code></li>
*  <li><code> revokeThread()</code></li>
* </ul> <p>
* <b> Nothing tested here. </b>
* @see com.sun.star.java.XJavaThreadRegister_11
*/
public class _XJavaThreadRegister_11 extends MultiMethodTest {
    public XJavaThreadRegister_11 oObj;

    protected void after() {
        log.println("Skipping all XJavaThreadRegister_11 methods, since they"
                + " can't be tested in the context");
        throw new StatusException(Status.skipped(true));
    }

    public void _isThreadAttached() {
        // skipping the test
        tRes.tested("isThreadAttached()", true);
    }

    public void _registerThread() {
        // skipping the test
        tRes.tested("registerThread()", true);
    }

    public void _revokeThread() {
        // skipping the test
        tRes.tested("revokeThread()", true);
    }
}
