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
    * Has <b>OK</b> status if no runtime exceptions occurred.
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
    * Has <b>OK</b> status if no runtime exceptions occurred.
    */
    public void _getAlignment() {

        boolean result = true ;
        align = oObj.getAlignment() ;

        tRes.tested("getAlignment()", result) ;
    }

}


