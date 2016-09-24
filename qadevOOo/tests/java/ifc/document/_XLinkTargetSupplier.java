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

package ifc.document;

import lib.MultiMethodTest;

import com.sun.star.container.XNameAccess;
import com.sun.star.document.XLinkTargetSupplier;

/**
* Testing <code>com.sun.star.document.XLinkTargetSupplier</code>
* interface methods :
* <ul>
*  <li><code> getLinks()</code></li>
* </ul> <p>
* @see com.sun.star.document.XLinkTargetSupplier
*/
public class _XLinkTargetSupplier extends MultiMethodTest {

    public XLinkTargetSupplier    oObj = null;

    /**
    * Just calls the method. <p>
    * Has <b> OK </b> status if not <code>null</code> value returned.
    */
    public void _getLinks() {
        XNameAccess links = oObj.getLinks();
        links.getElementNames();
        tRes.tested("getLinks()", true);
    }

}
