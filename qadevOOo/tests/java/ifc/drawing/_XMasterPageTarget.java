/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XMasterPageTarget.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:41:29 $
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

