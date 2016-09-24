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
import com.sun.star.container.XIndexAccess;
import com.sun.star.text.XFootnotesSupplier;

/**
 * Testing <code>com.sun.star.text.XFootnotesSupplier</code>
 * interface methods :
 * <ul>
 *  <li><code> getFootnotes()</code></li>
 *  <li><code> getFootnoteSettings()</code></li>
 * </ul> <p>
 * Test is <b> NOT </b> multithread compliant. <p>
 * @see com.sun.star.text.XFootnotesSupplier
 */
public class _XFootnotesSupplier extends MultiMethodTest {
    public XFootnotesSupplier oObj = null;

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns not
     * <code>null</code> value.
     */
    public void _getFootnotes(){
        boolean result = true;
        XIndexAccess footnotes = null;

        log.println("testing getFootnotes() ... ");

        footnotes = oObj.getFootnotes();
        result = footnotes != null;
        tRes.tested("getFootnotes()", result);
    }

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns not
     * <code>null</code> value.
     */
    public void _getFootnoteSettings(){
        boolean result = true;
        XPropertySet footnoteSettings = null;

        log.println("testing getFootnoteSettings() ... ");

        footnoteSettings = oObj.getFootnoteSettings();
        result = footnoteSettings != null;

        tRes.tested("getFootnoteSettings()", result);
    }
}

