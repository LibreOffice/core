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

import com.sun.star.drawing.XDrawPages;
import com.sun.star.drawing.XMasterPagesSupplier;

/**
* Testing <code>com.sun.star.drawing.XMasterPagesSupplier</code>
* interface methods :
* <ul>
*  <li><code> getMasterPages()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.drawing.XMasterPagesSupplier
*/
public class _XMasterPagesSupplier extends MultiMethodTest{
    public XMasterPagesSupplier oObj = null;

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the value returned is not null.
    */
    public void _getMasterPages(){
        boolean result = false;
        XDrawPages DrawPages = null;

        log.println("get the MasterPages");

        DrawPages = oObj.getMasterPages();
        result = DrawPages != null;

        tRes.tested("getMasterPages()",result);
    }
}

