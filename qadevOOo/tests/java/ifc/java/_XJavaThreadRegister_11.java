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

    @Override
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
