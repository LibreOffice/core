/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
* Test is <b> NOT </b> multithread compliant. <p>
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


