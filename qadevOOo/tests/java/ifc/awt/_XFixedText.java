/*************************************************************************
 *
 *  $RCSfile: _XFixedText.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:11:19 $
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

package ifc.awt;


import lib.MultiMethodTest;

import com.sun.star.awt.TextAlign;
import com.sun.star.awt.XFixedText;

/**
* Testing <code>com.sun.star.awt.XFixedText</code>
* interface methods :
* <ul>
*  <li><code> setText()</code></li>
*  <li><code> getText()</code></li>
*  <li><code> setAlignment()</code></li>
*  <li><code> getAlignment()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.awt.XFixedText
*/
public class _XFixedText extends MultiMethodTest {

    public XFixedText oObj = null;
    private String text = null ;
    private int align = -1 ;

    /**
    * Sets value changed and then compares it to get value. <p>
    * Has <b>OK</b> status if set and get values are equal.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getText </code>  </li>
    * </ul>
    */
    public void _setText() {
        requiredMethod("getText()") ;

        boolean result = true ;
        oObj.setText(text + "_") ;
        result = (text+"_").equals(oObj.getText()) ;

        tRes.tested("setText()", result) ;
    }

    /**
    * Just calls the method and stores value returned. <p>
    * Has <b>OK</b> status if no runtime exceptions occured.
    */
    public void _getText() {

        boolean result = true ;
        text = oObj.getText() ;
        if (util.utils.isVoid(text)) text = "XFixedText";

        tRes.tested("getText()", result) ;
    }

    /**
    * Sets value changed and then compares it to get value. <p>
    * Has <b>OK</b> status if set and get values are equal.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getAlignment </code>  </li>
    * </ul>
    */
    public void _setAlignment() {
        requiredMethod("getAlignment()") ;

        boolean result = true ;
        int newAlign = align ==
            TextAlign.CENTER ? TextAlign.LEFT : TextAlign.CENTER ;
        oObj.setAlignment((short)newAlign) ;
        short getAlign = oObj.getAlignment() ;
        result = newAlign == getAlign ;

        tRes.tested("setAlignment()", result) ;
    }

    /**
    * Just calls the method and stores value returned. <p>
    * Has <b>OK</b> status if no runtime exceptions occured.
    */
    public void _getAlignment() {

        boolean result = true ;
        align = oObj.getAlignment() ;

        tRes.tested("getAlignment()", result) ;
    }

}


