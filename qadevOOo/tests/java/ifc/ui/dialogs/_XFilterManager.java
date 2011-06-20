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
    * Has <b>OK</b> status if no exceptions occurred, else one of
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


