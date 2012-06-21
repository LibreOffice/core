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

import com.sun.star.beans.XPropertySet;
import com.sun.star.document.XDocumentInfo;
import com.sun.star.document.XDocumentInfoSupplier;
import com.sun.star.uno.UnoRuntime;
import lib.MultiMethodTest;

/**
 * test the XDocumentInfoSupplier interface.
 */
public class _XDocumentInfoSupplier extends MultiMethodTest {
    public XDocumentInfoSupplier oObj = null;

    public void _getDocumentInfo() {
        // returns a com.sun.star.document.DocumentInfo
        XDocumentInfo info = oObj.getDocumentInfo();
        XPropertySet xPropertySet = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, info);
        String title = null;
        try {
            title = (String)xPropertySet.getPropertyValue("Title");
        }
        catch(Exception e) {
            e.printStackTrace((java.io.PrintWriter)log);
            title = null;
        }
        log.println("Got document title: " + title);
        tRes.tested("getDocumentInfo()", title != null);
    }
}
