/*************************************************************************
 *
 *  $RCSfile: _XDrawPages.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:30:55 $
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

/**
* Testing <code>com.sun.star.drawing.XDrawPages</code>
* interface methods :
* <ul>
*  <li><code> insertNewByIndex()</code></li>
*  <li><code> remove()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.drawing.XDrawPages
*/
public class _XDrawPages extends MultiMethodTest {
    public XDrawPages oObj = null;
    public XDrawPage DrawPage;

    /**
    * Inserts a draw page at index 0. Number of pages is obtained
    * before and after insertion.<p>
    * Has <b> OK </b> status if the value returned is not null and
    * number of pages increases by one after insertion. <p>
    */
    public void _insertNewByIndex(){
        boolean result = true;
        log.println("testing insertNewByIndex() ... ");

        int cntBefore = oObj.getCount();
        DrawPage = oObj.insertNewByIndex(0);
        int cntAfter = oObj.getCount();
        result = DrawPage != null &&
            cntAfter == cntBefore + 1 ;

        tRes.tested("insertNewByIndex()", result);
    } // end InsertNewByIndex

    /**
    * Removes the draw page inserted before. Number of pages is obtained
    * before and after removing.<p>
    * Has <b> OK </b> status if the number of pages decreases by one
    * after removing. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> insertByIndex() </code> : the page to be removed must
    *   be inserted first.</li>
    * </ul>
    */
    public void _remove(){
        requiredMethod("insertNewByIndex()") ;

        boolean result = true;
        log.println("removing DrawPage...");

        int cntBefore = oObj.getCount();
        oObj.remove(DrawPage);
        int cntAfter = oObj.getCount();
        result = cntAfter + 1 == cntBefore ;

        tRes.tested("remove()", result);
    } // end remove()

} // end XDrawPages

