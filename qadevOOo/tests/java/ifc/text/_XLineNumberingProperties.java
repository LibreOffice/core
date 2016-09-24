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

package ifc.text;

import lib.MultiMethodTest;

import com.sun.star.beans.XPropertySet;
import com.sun.star.text.XLineNumberingProperties;

/**
 * Testing <code>com.sun.star.text.XLineNumberingProperties</code>
 * interface methods :
 * <ul>
 *  <li><code> getLineNumberingProperties()</code></li>
 * </ul> <p>
 * Test is <b> NOT </b> multithread compliant. <p>
 * @see com.sun.star.text.XLineNumberingProperties
 */
public class _XLineNumberingProperties extends MultiMethodTest {

    public XLineNumberingProperties oObj = null;

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns not
     * <code>null</code> value.
     */
    public void _getLineNumberingProperties() {

        XPropertySet getting = oObj.getLineNumberingProperties();
        tRes.tested("getLineNumberingProperties()",getting != null);
    }

}  // finish class _XLineNumberingProperties


