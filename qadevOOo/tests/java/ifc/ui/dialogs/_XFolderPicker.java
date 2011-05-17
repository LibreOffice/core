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

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ui.dialogs.XFolderPicker;

/**
* Testing <code>com.sun.star.ui.XFolderPicker</code>
* interface methods :
* <ul>
*  <li><code> setDisplayDirectory()</code></li>
*  <li><code> getDisplayDirectory()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.ui.XFolderPicker
*/
public class _XFolderPicker extends MultiMethodTest {

    public XFolderPicker oObj = null;
    private String dir = null ;

    /**
    * Sets the current directory to SOffice temp dir. <p>
    * Has <b>OK</b> status if no exceptions occurred.
    */
    public void _setDisplayDirectory() {
        boolean result = true ;
        dir = util.utils.getOfficeTemp((XMultiServiceFactory)tParam.getMSF()) ;

        log.println("Trying to set dir '" + dir + "'") ;
        try {
            oObj.setDisplayDirectory(dir) ;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Directory '" + dir + "' not found :" + e) ;
            result = false ;
        }

        tRes.tested("setDisplayDirectory()", result) ;
    }

    /**
    * Gets the current directory. <p>
    * Has <b>OK</b> status if get value is equal to set value
    * passed to <code>setDisplayDirectory</code> <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setDisplayDirectory </code>  </li>
    * </ul>
    */
    public void _getDisplayDirectory() {
        requiredMethod("setDisplayDirectory()") ;

        String gDir = oObj.getDisplayDirectory() ;

        log.println("Get dir '" + gDir + "'") ;

        tRes.tested("getDisplayDirectory()", dir.equals(gDir)) ;
    }

    /**
    * Gets the directory chosen by the user. <p>
    * Has <b>OK</b> status if get value is not NULL <p>
    */
    public void _getDirectory() {

        String gDir = oObj.getDirectory() ;

        log.println("Get dir '" + gDir + "'") ;

        tRes.tested("getDirectory()", gDir != null) ;
    }

    /**
    * Sets the Description for the dialog. <p>
    * Has <b>OK</b> status if no error occurs <p>
    */
    public void _setDescription() {

        oObj.setDescription("XFolderPicker") ;
        //to visually check if the method works
        //oObj.execute();
        tRes.tested("setDescription()", true) ;
    }

}


