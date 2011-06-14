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
import com.sun.star.drawing.XDrawView;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;

/**
* Testing <code>com.sun.star.drawing.XDrawView</code>
* interface methods :
* <ul>
*  <li><code> setCurrentPage()</code></li>
*  <li><code> getCurrentPage()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'Pages'</code> (of type <code>XDrawPages</code>):
*   needed to have the access to pages collection.</li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.drawing.XDrawView
*/
public class _XDrawView extends MultiMethodTest {

    public XDrawView oObj = null;
    public XDrawPage the_page = null;

    /**
     * This methods gets the current DrawPage.<p>
     * Has <b> OK </b> status if the returned DrawPage
     * isn't empty.
     */
    public void _getCurrentPage(){
        the_page = oObj.getCurrentPage();
        tRes.tested("getCurrentPage()",the_page != null);
    } // end getCurrentPage

    /**
     * This methods sets the current DrawPage<br>
     * First a new DrawPage is inserted in the document.
     * Then this DrawPage is set as current Page.
     * Has <b> OK </b> status if the getCurrentPage() method returns
     * the DrawPage that was previously set.
     * @see ifc.drawing._XDrawPages
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> getCurrentPage() </code> </li>
     * </ul>
     */
    public void _setCurrentPage(){
        requiredMethod("getCurrentPage()");
        try {
            XDrawPages the_pages = (XDrawPages) tEnv.getObjRelation("Pages");
            the_pages.insertNewByIndex(0);
            XDrawPage newPage = (XDrawPage) AnyConverter.toObject(
                            new Type(XDrawPage.class),the_pages.getByIndex(1));
            oObj.setCurrentPage(newPage);
            XDrawPage getting = oObj.getCurrentPage();
            boolean eq = newPage.equals(getting);
            if (!eq) {
                log.println("Getting: "+getting.hasElements());
                log.println("Expected: "+newPage.hasElements());
            }
            //back to the previous page
            oObj.setCurrentPage(the_page);
            tRes.tested("setCurrentPage()",eq);
        } catch (com.sun.star.lang.WrappedTargetException ex) {
            log.println("Exception occurred while checking 'setCurrentPage()'");
            ex.printStackTrace(log);
            tRes.tested("setCurrentPage()",false);
        } catch (com.sun.star.lang.IndexOutOfBoundsException ex) {
            log.println("Exception occurred while checking 'setCurrentPage()'");
            ex.printStackTrace(log);
            tRes.tested("setCurrentPage()",false);
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("Exception occurred while checking 'setCurrentPage()'");
            ex.printStackTrace(log);
            tRes.tested("setCurrentPage()",false);
        }
    } // end setCurrentPage

} // end DrawView

