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
import com.sun.star.text.XEndnotesSupplier;


/**
 * Testing <code>com.sun.star.text.XEndnotesSupplier</code>
 * interface methods :
 * <ul>
 *  <li><code> getEndnotes()</code></li>
 *  <li><code> getEndnoteSettings()</code></li>
 * </ul> <p>
 * Test is <b> NOT </b> multithread compliant. <p>
 * @see com.sun.star.text.XEndnotesSupplier
 */
public class _XEndnotesSupplier extends MultiMethodTest {

    public static XEndnotesSupplier oObj = null;        // oObj filled by MultiMethodTest

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the collection returned has at least
     * one element.
     */
    public void _getEndnotes() {
        boolean res = false;

        XIndexAccess the_endnotes = oObj.getEndnotes();
        res = the_endnotes.hasElements();

        tRes.tested("getEndnotes()",res);
    }

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns not
     * <code>null</code> value.
     */
    public void _getEndnoteSettings() {
        boolean res = false;

        XPropertySet the_props = oObj.getEndnoteSettings();
        res = ( the_props != null );

        tRes.tested("getEndnoteSettings()",res);
    }

}  // finish class _XEndnotesSupplier


