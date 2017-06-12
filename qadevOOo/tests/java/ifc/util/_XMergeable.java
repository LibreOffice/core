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

package ifc.util;

import lib.MultiMethodTest;

import com.sun.star.util.XMergeable;


/**
 * Testing <code>com.sun.star.util.XMergeable</code>
 * interface methods :
 * <ul>
 *  <li><code> merge()</code></li>
 *  <li><code> getIsMerged()</code></li>
 * </ul> <p>
 * Test is <b> NOT </b> multithread compliant. <p>
 * @see com.sun.star.util.XMergeable
 */
public class _XMergeable extends MultiMethodTest {

    public XMergeable oObj = null;                // oObj filled by MultiMethodTest

    /**
     * Checks the property <code>IsMerged</code> and sets it
     * to the opposite value. <p>
     * Has <b> OK </b> status if the property value has changed. <p>
     */
    public void _getIsMerged () {

        boolean isMerged = oObj.getIsMerged();
        oObj.merge(!isMerged);
        tRes.tested("getIsMerged()", isMerged != oObj.getIsMerged());
    }

    /**
     * Checks the property <code>IsMerged</code> and sets it
     * to the opposite value. <p>
     * Has <b> OK </b> status if the property value has changed. <p>
     */
    public void _merge () {

        boolean isMerged = oObj.getIsMerged();
        oObj.merge(!isMerged);
        tRes.tested("merge()", isMerged != oObj.getIsMerged());
    }
} // finish class _XMergeable


