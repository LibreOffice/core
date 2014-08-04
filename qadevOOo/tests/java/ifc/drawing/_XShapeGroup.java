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

package ifc.drawing;

import lib.MultiMethodTest;

import com.sun.star.drawing.XShapeGroup;

/**
* Testing <code>com.sun.star.drawing.XShapeGroup</code>
* interface methods :
* <ul>
*  <li><code> enterGroup()</code></li>
*  <li><code> leaveGroup()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.drawing.XShapeGroup
*/
public class _XShapeGroup extends MultiMethodTest {
    public XShapeGroup oObj = null;

    /**
    * The method called.
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown.
    */
    public void _enterGroup() {
        log.println("Testing enterGroup()...");
        oObj.enterGroup();
        tRes.tested("enterGroup()", true);
    }

    /**
    * The method called.
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown.
    */
    public void _leaveGroup() {
        log.println("Testing leaveGroup()...");
        oObj.leaveGroup();
        tRes.tested("leaveGroup()", true);
    }

} // end of _XShapeGroup

