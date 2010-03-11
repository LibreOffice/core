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

package ifc.drawing;

import lib.MultiMethodTest;

import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPages;
import com.sun.star.drawing.XMasterPageTarget;

/**
* Testing <code>com.sun.star.drawing.XMasterPageTarget</code>
* interface methods :
* <ul>
*  <li><code> getMasterPage()</code></li>
*  <li><code> setMasterPage()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'MasterPageSupplier'</code>
*   (of type <code>XDrawPages</code>):
*    the relation is used to create a new draw page. </li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.drawing.XMasterPageTarget
*/
public class _XMasterPageTarget extends MultiMethodTest{

    public XMasterPageTarget oObj = null;
    public XDrawPage DrawPage = null;

    /**
    * Gets the master page and stores. <p>
    * Has <b> OK </b> status if the value returned is not null. <p>
    */
    public void _getMasterPage(){
        boolean result = false;
        log.println("get the MasterPage");

        DrawPage = oObj.getMasterPage();
        result = DrawPage != null ;

        tRes.tested("getMasterPage()",result);
    }

    /**
    * Gets the pages supplier from relation. Then a new page is created,
    * inserted and set as master page.<p>
    * Has <b> OK </b> status if the master page get is equal to page
    * which was set. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getMasterPage() </code> : to get old master page.</li>
    * </ul>
    */
    public void _setMasterPage() {
        requiredMethod("getMasterPage()") ;

        boolean result = true;
        XDrawPage oNewPage = null;

        XDrawPages oPages = (XDrawPages) tEnv.getObjRelation("MasterPageSupplier");
        log.println("inserting new MasterPage");
        oNewPage = oPages.insertNewByIndex(oPages.getCount());

        // save the old MasterPage
        XDrawPage oOldPage = DrawPage;

        // set the new Page as MasterPage
        log.println("set the new MasterPage");
        oObj.setMasterPage(oNewPage);

        // get the new MasterPage
        DrawPage = oObj.getMasterPage();

        // test the diffrent MasterPages
        if (DrawPage.equals(oOldPage)) result = false;

        tRes.tested("setMasterPage()",result);
    }

} // end of XMasterPageTarget

