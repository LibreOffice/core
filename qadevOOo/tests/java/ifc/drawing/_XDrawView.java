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
* Test is <b> NOT </b> multithread compliant. <p>
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

