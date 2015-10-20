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

import com.sun.star.document.XMimeTypeInfo;
import com.sun.star.lang.XComponent;

/**
 * Testing <code>com.sun.star.document.XMimeTypeInfo</code>
 * interface methods :
 * <ul>
 *  <li><code> supportsMimeType()</code></li>
 *  <li><code> getSupportedMimeTypeNames()</code></li>
 * </ul> <p>
 *
 * @see com.sun.star.document.XMimeTypeInfo
 */
public class _XMimeTypeInfo extends MultiMethodTest {

    public XMimeTypeInfo oObj = null;
    public String[] smi = null;

    /**
    * Gets supported types and stores them. <p>
    * Has <b> OK </b> status if at least one type exists.
    */
    public void _getSupportedMimeTypeNames() {
        smi = oObj.getSupportedMimeTypeNames();
        tRes.tested("getSupportedMimeTypeNames()", smi.length>0) ;
    }

    /**
     * Calls the method for one supported type retrieved by
     * <code>getSupportedMimeTypeNames</code> method and for
     * bad type. <p>
     *
     * Has <b> OK </b> status if <code>true</code> returned for
     * supported type and <code>false</code> for bad type.
     *
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> getSupportedMimeTypeNames </code> : to have a list of
     *    supported types. </li>
     * </ul>
     */
    public void _supportsMimeType() {
        requiredMethod("getSupportedMimeTypeNames()");
        boolean pos = false;
        pos = oObj.supportsMimeType(smi[0]);
        if (!pos) {
            log.println("Method returns false for existing MimeType");
        }
        boolean neg = true;
        neg = oObj.supportsMimeType("NoRealMimeType");
        if (neg) {
            log.println("Method returns true for non existing MimeType");
        }
        tRes.tested("supportsMimeType()", (pos && !neg)) ;
    }
}


