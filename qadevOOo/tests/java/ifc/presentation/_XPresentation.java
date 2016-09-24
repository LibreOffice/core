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

package ifc.presentation;

import lib.MultiMethodTest;

import com.sun.star.presentation.XPresentation;

/**
* Testing <code>com.sun.star.presentation.XPresentation</code>
* interface methods :
* <ul>
*  <li><code> start()</code></li>
*  <li><code> end()</code></li>
*  <li><code> rehearseTimings()</code></li>
* </ul> <p>
* @see com.sun.star.presentation.XPresentation
*/
public class _XPresentation extends MultiMethodTest {

    public XPresentation oObj = null;

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns. <p>
    */
    public void _start() {
        oObj.start();
        tRes.tested("start()", true);
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> start() </code> : presentation must be started before</li>
    * </ul>
    */
    public void _end() {
        requiredMethod("start()");

        oObj.end();
        tRes.tested("end()", true);
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns. <p>
    */
    public void _rehearseTimings() {
        oObj.rehearseTimings();
        tRes.tested("rehearseTimings()", true);
    }

}  // finish class _XPresentation


