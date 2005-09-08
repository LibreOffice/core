/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XSheetLinkable.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:56:22 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
