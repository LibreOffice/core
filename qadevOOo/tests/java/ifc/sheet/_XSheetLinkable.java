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
package ifc.sheet;

import com.sun.star.sheet.SheetLinkMode;
import com.sun.star.sheet.XSheetLinkable;
import lib.MultiMethodTest;
import util.utils;

/**
 *
 */
public class _XSheetLinkable extends MultiMethodTest {
    public XSheetLinkable oObj = null;
    SheetLinkMode slm = null;
    String linkSheetName = null;
    String linkUrl = null;
    String linkUrl2 = null;

    public void before() {
        // get a document for linking.
        linkUrl = (String)tEnv.getObjRelation("XSheetLinkable.LinkSheet");
        linkUrl = utils.getFullTestURL(linkUrl);

        // get a second sheet for changing the link url: if it's not set,
        // this part of the test is omitted.
        linkUrl2 = (String)tEnv.getObjRelation("XSheetLinkable.LinkSheet2");
        if (linkUrl2 != null)
            linkUrl = utils.getFullTestURL(linkUrl);

        // set a name for the sheet.
        linkSheetName = "Sheet1";
    }

    public void _getLinkMode() {
        requiredMethod("link()");
        slm = oObj.getLinkMode();
        tRes.tested("getLinkMode()", slm == SheetLinkMode.VALUE);
    }

    public void _getLinkSheetName() {
        requiredMethod("link()");
        String lSheetName = oObj.getLinkSheetName();
        tRes.tested("getLinkSheetName()", linkSheetName.equals(lSheetName));
    }

    public void _getLinkUrl() {
        requiredMethod("link()");
        String lUrl = oObj.getLinkUrl();
        System.out.println("URL: " + lUrl);
        tRes.tested("getLinkUrl()", lUrl.equals(linkUrl));
    }

    public void _link() {
        oObj.link(linkUrl, linkSheetName, "", "", SheetLinkMode.VALUE);
        tRes.tested("link()", true);
    }

    public void _setLinkMode() {
        requiredMethod("getLinkMode()");
        oObj.setLinkMode(SheetLinkMode.NONE);
        slm = oObj.getLinkMode();
        tRes.tested("setLinkMode()", slm == SheetLinkMode.NONE);
    }

    public void _setLinkSheetName() {
        requiredMethod("getLinkSheetName()");
        oObj.setLinkSheetName("Sheet2");
        linkSheetName = oObj.getLinkSheetName();
        tRes.tested("setLinkSheetName()", linkSheetName.equals("Sheet2"));
    }

    public void _setLinkUrl() {
        requiredMethod("getLinkUrl()");
        boolean result = false;
        if (linkUrl2 == null) {
            // set back to the original value
            oObj.setLinkUrl(linkUrl);
            result = true;
        }
        else {
            oObj.setLinkUrl(linkUrl2);
            linkUrl = oObj.getLinkUrl();
            result = linkUrl.equals(linkUrl2);
        }
        tRes.tested("setLinkUrl()", result);
    }
}
