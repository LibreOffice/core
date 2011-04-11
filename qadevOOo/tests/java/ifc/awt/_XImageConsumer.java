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

import com.sun.star.awt.XImageConsumer;

/**
* Testing <code>com.sun.star.awt.XImageConsumer</code>
* interface methods :
* <ul>
*  <li><code> init()</code></li>
*  <li><code> setColorModel()</code></li>
*  <li><code> setPixelsByBytes()</code></li>
*  <li><code> setPixelsByLongs()</code></li>
*  <li><code> complete()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.awt.XImageConsumer
*/

public class _XImageConsumer extends MultiMethodTest {

    public XImageConsumer oObj = null;

    /**
    * Initialize the consumer with size 2x2. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred
    */
    public void _init() {

        boolean result = true ;
        oObj.init(2, 2) ;

        tRes.tested("init()", result) ;
    }

    /**
    * Sets color model. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> init </code> </li>
    * </ul>
    */
    public void _setColorModel() {
        requiredMethod("init()") ;

        boolean result = true ;
        int[] pal = new int[256] ;
        for (int i = 0; i < 256; i++) pal[i] = i ;
        oObj.setColorModel((short)8, pal, 100, 100, 100, 100) ;

        tRes.tested("setColorModel()", result) ;
    }

    /**
    * Fill the picture with for pixels. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred
    * The following method tests are to be executed before :
    * <ul>
    *  <li> <code> setColorModel </code> </li>
    * </ul>
    */
    public void _setPixelsByBytes() {
        executeMethod("setColorModel()") ;

        boolean result = true ;
        oObj.setPixelsByBytes(0, 0, 2, 2,
            new byte[] {(byte)0, (byte)255, (byte)255, (byte)0}, 0, 2) ;

        tRes.tested("setPixelsByBytes()", result) ;
    }

    /**
    * Fill the picture with for pixels. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred
    * The following method tests are to be executed before :
    * <ul>
    *  <li> <code> setColorModel </code> </li>
    * </ul>
    */
    public void _setPixelsByLongs() {
        executeMethod("setColorModel()") ;

        boolean result = true ;
        oObj.setPixelsByLongs(0, 0, 2, 2, new int[] {0, 255, 255, 0}, 0, 2) ;

        tRes.tested("setPixelsByLongs()", result) ;
    }

    /**
    * Just calls the method. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> init </code>  </li>
    * </ul> <p>
    * The following method tests are to be executed before :
    * <ul>
    *  <li> <code> setPixelsByBytes </code>  </li>
    *  <li> <code> setPixelsByBytes </code>  </li>
    * </ul>
    */
    public void _complete() {
        requiredMethod("init()") ;
        executeMethod("setPixelsByBytes()") ;
        executeMethod("setPixelsByBytes()") ;

        boolean result = true ;
        oObj.complete(0, null) ;

        tRes.tested("complete()", result) ;
    }
}


