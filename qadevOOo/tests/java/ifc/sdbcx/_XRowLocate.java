/*************************************************************************
 *
 *  $RCSfile: _XRowLocate.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:56:30 $
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

package ifc.sdbcx;

import lib.MultiMethodTest;

import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbcx.XRowLocate;

/**
* Testing <code>com.sun.star.sdbcx.XRowLocate</code>
* interface methods :
* <ul>
*  <li><code> getBookmark()</code></li>
*  <li><code> moveToBookmark()</code></li>
*  <li><code> moveRelativeToBookmark()</code></li>
*  <li><code> compareBookmarks()</code></li>
*  <li><code> hasOrderedBookmarks()</code></li>
*  <li><code> hashBookmark()</code></li>
* </ul> <p>
* Object relations needed :
* <ul>
* <li> <code>XRowLocate.HasOrderedBookmarks</code> <b>(optional)</b>:
*  <code>Boolean</code> type. If information about bookmark ordering
*  of object tested exists it placed in this relation.</li>
* </ul> <p>
* @see com.sun.star.sdbcx.XRowLocate
*/
public class _XRowLocate extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XRowLocate oObj = null ;

    private Object bookmark1 = null, bookmark2 = null ;

    /**
    * Gets the bookmark of the current cursor position and
    * stores it for other methods use.
    * Has OK status if method returns not null
    * <code>XNameAccess</code> object, FAILED otherwise.
    */
    public void _getBookmark() {

        try {
            bookmark1 = oObj.getBookmark() ;
        } catch (SQLException e) {
            log.println("Exception occured :" + e) ;
        }

        tRes.tested("getBookmark()", bookmark1 != null) ;
    }

    /**
    * Moves to bookmark previously created by method <code>_getBookmark</code>
    * then creates new bookmark and compare it to the first one. The
    * row difference must be equal to 0. <p>
    * Method tests to be completed successfully before :
    * <ul>
    * <li> <code>getBookmark()</code> : to have a bookmark to move to.</li>
    * </ul>
    * Method tests to be executed before :
    * <ul>
    * <li> <code>moveRelativeToBookmark()</code> : to move the cursor from
    *    the initial position, to check if it returns back.</li>
    * </ul> <p>
    * Has OK status difference between positions of bookmarks (where
    * cursor was moved to and created new one) equals to 0 and no
    * exceptions occured, FAILED otherwise.
    */
    public void _moveToBookmark() {
        requiredMethod("getBookmark()") ;
        executeMethod("moveRelativeToBookmark()") ;

        int comparison = -1 ;
        Object tmpBookmark = null ;
        try {
            synchronized(oObj) {
                oObj.moveToBookmark(bookmark1) ;
                tmpBookmark = oObj.getBookmark() ;
            }
            comparison = oObj.compareBookmarks(bookmark1, tmpBookmark) ;
        } catch (SQLException e) {
            log.println("Exception occured :" + e) ;
        }

        tRes.tested("moveToBookmark()", comparison == 0) ;
    }

    /**
    * Moves the cursor to the position with offset 2 from the
    * bookmark created by <code>_getBookmark()</code> method and
    * then creates a bookmark of new position. <p>
    * Method tests to be completed successfully before :
    * <ul>
    * <li> <code>getBookmark()</code> : to have a bookmark to move to.</li>
    * </ul> <p>
    * Has OK status if no exceptions occured while method call.
    */
    public void _moveRelativeToBookmark() {
        requiredMethod("getBookmark()") ;

        boolean result = true ;
        try {
            synchronized (oObj) {
                oObj.moveRelativeToBookmark(bookmark1, 1) ;
                bookmark2 = oObj.getBookmark() ;
            }
        } catch (SQLException e) {
            log.println("Exception occured :" + e) ;
            result = false ;
        }

        tRes.tested("moveRelativeToBookmark()", result) ;
    }

    /**
    * Compares bookmarks created in <code>getBookmark()</code> and
    * <code>moveRelativeToBokkmark()</code> methods.
    * Method tests to be completed successfully before :
    * <ul>
    * <li> <code>getBookmark()</code> : to have first bookmark to compare.</li>
    * <li> <code>moveRelativeToBookmark()</code> : to have second
    *   bookmark to compare.</li>
    * </ul> <p>
    * Has OK status if difference in bookmark positions equals to 2.
    */
    public void _compareBookmarks() {
        requiredMethod("getBookmark()") ;
        requiredMethod("moveRelativeToBookmark()") ;

        int comparison = 0 ;
        int comparison1 = 0 ;
        try {
            comparison = oObj.compareBookmarks(bookmark1, bookmark2) ;
            comparison1 = oObj.compareBookmarks(bookmark1, bookmark1) ;
        } catch (SQLException e) {
            log.println("Exception occured :" + e) ;
        }

        if (comparison != -1) {
            log.println("! Wrong compare number :" + comparison) ;
        }

        tRes.tested("compareBookmarks()", comparison == -1 && comparison1 == 0) ;
    }

    /**
    * Calls method. If relation is not found, returned result is not
    * checked. <p>
    * Has OK status if method returned right value (in case if relation
    * and value to compare to exist), or just successfully returned
    * (in case of no relation).
    */
    public void _hasOrderedBookmarks() {
        boolean result = true ;
        boolean res = true ;

        // Optional relation
        Boolean has = (Boolean) tEnv.getObjRelation
            ("XRowLocate.HasOrderedBookmarks") ;

        try {
            res = oObj.hasOrderedBookmarks() ;
        } catch (SQLException e) {
            log.println("Exception occured :" + e) ;
            result = false ;
        }

        if (has == null) {
            log.println("Optional relation not found - result not checked") ;
        } else {
            result &= res == has.booleanValue() ;
        }

        tRes.tested("hasOrderedBookmarks()", result) ;
    }

    /**
    * Gets hash values of two previously created bookmarks of different
    * cursor positions and compares them. <p>
    * Method tests to be completed successfully before :
    * <ul>
    * <li> <code>getBookmark()</code> : to have first bookmark.</li>
    * <li> <code>moveRelativeToBookmark()</code> : to have second
    *   bookmark.</li>
    * </ul> <p>
    * Has OK status if hash values of two bookmarks are different.
    */
    public void _hashBookmark() {
        requiredMethod("getBookmark()") ;
        requiredMethod("moveRelativeToBookmark()") ;

        boolean result = true ;
        try {
            int hash1 = oObj.hashBookmark(bookmark1) ;
            int hash2 = oObj.hashBookmark(bookmark2) ;
            log.println("1st hash = " + hash1 + ", 2nd = " + hash2) ;

            result = hash1 != hash2 ;
        } catch (SQLException e) {
            log.println("Exception occured :" + e) ;
            result = false ;
        }

        tRes.tested("hashBookmark()", result) ;
    }

}  // finish class _XRowLocate

