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

    @Override
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
