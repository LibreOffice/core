/*************************************************************************
 *
 *  $RCSfile: _XDrawView.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 10:31:16 $
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
            log.println("Exception occured while checking 'setCurrentPage()'");
            ex.printStackTrace(log);
            tRes.tested("setCurrentPage()",false);
        } catch (com.sun.star.lang.IndexOutOfBoundsException ex) {
            log.println("Exception occured while checking 'setCurrentPage()'");
            ex.printStackTrace(log);
            tRes.tested("setCurrentPage()",false);
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("Exception occured while checking 'setCurrentPage()'");
            ex.printStackTrace(log);
            tRes.tested("setCurrentPage()",false);
        }
    } // end setCurrentPage

} // end DrawView

