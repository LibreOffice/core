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

import com.sun.star.ui.dialogs.XFilePicker;

/**
* Testing <code>com.sun.star.ui.XFilePicker</code>
* interface methods :
* <ul>
*  <li><code> setMultiSelectionMode()</code></li>
*  <li><code> setDefaultName()</code></li>
*  <li><code> setDisplayDirectory()</code></li>
*  <li><code> getDisplayDirectory()</code></li>
*  <li><code> getPath()</code></li>
* </ul> <p>
* The following predefined files needed to complete the test:
* <ul>
*  <li> <code>'space-metal.jpg'</code> : just to exist. </li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.ui.XFolderPicker
*/
public class _XFilePicker extends MultiMethodTest {

    public XFilePicker oObj = null;
    private String dir = null ;
    private String fname = "space-metal.jpg" ;

    /**
    * Sets the current directory to the test document directory. <p>
    * Has <b>OK</b> status if no exceptions occurred.
    */
    public void _setDisplayDirectory() {
        boolean result = true ;
        dir = util.utils.getFullTestURL("") ;

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
    * Sets default name to file name existing in test document
    * directory ('space-metal.jpg'). <p>
    * Has <b>OK</b> status if no exceptions occurred.
    */
    public void _setDefaultName() {
        boolean result = true ;

        try {
            oObj.setDefaultName(fname) ;
        } catch (Exception e) {
            log.println("Exception setting default name :" + e) ;
            result = false ;
        }

        tRes.tested("setDefaultName()", result) ;
    }

    /**
    * Just switch object to MultiSelectionMode. There is no ways
    * to check this method (only interactively). <p>
    * Has <b>OK</b> status if no runtime exceptions occurred.
    */
    public void _setMultiSelectionMode() {

        oObj.setMultiSelectionMode(true) ;

        tRes.tested("setMultiSelectionMode()", true) ;
    }

    /**
    * Gets completed path from dialog. If <code>execute()</code>
    * method was not called then zero length array is returned.
    * So to check actual functionality of this method interactive
    * testing is required. <p>
    * Has <b>OK</b> status if zero length array returned (this
    * occurs if <code>execute()</code>
    * method was not called yet) or if array contains at least one
    * element and it equals to <code>[Directory set] +
    * [Default file name set]</code>. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setDisplayDirectory </code> </li>
    *  <li> <code> setDefaultName </code> </li>
    * </ul>
    */
    public void _getFiles() {
        requiredMethod("setDisplayDirectory()");
        requiredMethod("setDefaultName()");

        String[] files = oObj.getFiles();

        if (files.length > 0) {
            log.println("Path get : '" + files[0] + "'") ;

            tRes.tested("getFiles()", (dir + fname).equals(files[0])) ;
        } else {
            log.println("No files were selected or execute() method was not called.") ;

            tRes.tested("getFiles()", true);
        }
    }

}


