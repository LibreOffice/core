/*************************************************************************
 *
 *  $RCSfile: _XSheetLinkable.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Date: 2004-11-02 11:57:49 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
