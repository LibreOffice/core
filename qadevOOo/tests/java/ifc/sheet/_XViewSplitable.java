/*************************************************************************
 *
 *  $RCSfile: _XViewSplitable.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:07:44 $
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

import lib.MultiMethodTest;

import com.sun.star.sheet.XViewSplitable;

/**
* Testing <code>com.sun.star.sheet.XViewSplitable</code>
* interface methods :
* <ul>
*  <li><code> getIsWindowSplit()</code></li>
*  <li><code> getSplitHorizontal()</code></li>
*  <li><code> getSplitVertical()</code></li>
*  <li><code> getSplitColumn()</code></li>
*  <li><code> getSplitRow()</code></li>
*  <li><code> splitAtPosition()</code></li>
* </ul> <p>
* @see com.sun.star.sheet.XViewSplitable
*/
public class _XViewSplitable extends MultiMethodTest {

    public XViewSplitable oObj = null;
    protected final int posX = 101, posY = 51;

    /**
    * Test just calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns. <p>
    */
    public void _splitAtPosition(){
        oObj.splitAtPosition(posX, posY);
        tRes.tested("splitAtPosition()", true);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value is equal to true. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> splitAtPosition() </code> : that the view was splitted into
    *   individual panes </li>
    * </ul>
    */
    public void _getIsWindowSplit(){
        requiredMethod("splitAtPosition()");
        tRes.tested("getIsWindowSplit()", oObj.getIsWindowSplit());
    }

    /**
    * Test calls the method and compares returned value with value that was
    * set by method <code>splitAtPosition()</code>. <p>
    * Has <b> OK </b> status if returned value is equal to value that was
    * set by method <code>splitAtPosition()</code>. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> splitAtPosition() </code> : to split the view at the
    *  specified position </li>
    * </ul>
    */
    public void _getSplitHorizontal() {
        requiredMethod("splitAtPosition()");
        tRes.tested("getSplitHorizontal()", oObj.getSplitHorizontal() == posX);
    }

    /**
    * Test calls the method and compares returned value with value that was
    * set by method <code>splitAtPosition()</code>. <p>
    * Has <b> OK </b> status if returned value is equal to value that was
    * set by method <code>splitAtPosition()</code>. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> splitAtPosition() </code> : to split the view at the
    *  specified position </li>
    * </ul>
    */
    public void _getSplitVertical() {
        requiredMethod("splitAtPosition()");
        tRes.tested("getSplitVertical()", oObj.getSplitVertical() == posY);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't equal zero. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> splitAtPosition() </code> : to split the view at the
    *  specified position </li>
    * </ul>
    */
    public void _getSplitColumn() {
        requiredMethod("splitAtPosition()");
        tRes.tested("getSplitColumn()", oObj.getSplitColumn() != 0);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't equal zero. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> splitAtPosition() </code> : to split the view at the
    *  specified position </li>
    * </ul>
    */
    public void _getSplitRow() {
        requiredMethod("splitAtPosition()");
        tRes.tested("getSplitRow()", oObj.getSplitRow() != 0);
    }
}  // finish class _XViewSplitable


