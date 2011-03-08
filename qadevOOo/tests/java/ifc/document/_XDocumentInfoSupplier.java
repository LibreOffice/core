/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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
