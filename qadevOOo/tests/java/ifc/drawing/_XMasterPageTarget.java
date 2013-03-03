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

        // test the different MasterPages
        if (DrawPage.equals(oOldPage)) result = false;

        tRes.tested("setMasterPage()",result);
    }

} // end of XMasterPageTarget

