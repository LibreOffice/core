/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XFolderPicker.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:38:50 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
    * Has <b>OK</b> status if no exceptions occured.
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


