/*************************************************************************
 *
 *  $RCSfile: _XFilterManager.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:28:13 $
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

package ifc.ui.dialogs;

import lib.MultiMethodTest;

import com.sun.star.ui.dialogs.XFilterManager;

/**
* Testing <code>com.sun.star.ui.XFilterManager</code>
* interface methods :
* <ul>
*  <li><code> appendFilter()</code></li>
*  <li><code> setCurrentFilter()</code></li>
*  <li><code> getCurrentFilter()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.ui.XFilterManager
*/
public class _XFilterManager extends MultiMethodTest {

    public XFilterManager oObj = null;

    /**
    * Appends a new filter (for extension 'txt'). <p>
    * Has <b>OK</b> status if no runtime exceptions ocured.
    */
    public void _appendFilter() {
        boolean res = true;
        try {
            oObj.appendFilter("TestFilter", "txt");
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            res=false;
        }

        tRes.tested("appendFilter()", res) ;
    }

    /**
    * Sets the current filter to that which was appended before.<p>
    * Has <b>OK</b> status if no exceptions occured, else one of
    * <code>appendFilter</code> and <code>setCurrentFilter</code>
    * methods failed. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> appendFilter </code>  </li>
    * </ul>
    */
    public void _setCurrentFilter() {
        requiredMethod("appendFilter()") ;
        boolean result = true;

        try {
            oObj.setCurrentFilter("TestFilter") ;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("setCurrentFilter() or appendFilter() failed") ;
            result = false ;
        }

        tRes.tested("setCurrentFilter()", result) ;
    }

    /**
    * Gets current filter name and compares it filter name set before.<p>
    * Has <b>OK</b> status if set and get filter names are equal.<p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setCurrentFilter </code> </li>
    * </ul>
    */
    public void _getCurrentFilter() {
        requiredMethod("setCurrentFilter()") ;

        String gVal = oObj.getCurrentFilter() ;

        tRes.tested("getCurrentFilter()", "TestFilter".equals(gVal)) ;
    }
}


