/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XImageConsumer.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 22:58:11 $
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
    * Has <b> OK </b> status if no runtime exceptions occured
    */
    public void _init() {

        boolean result = true ;
        oObj.init(2, 2) ;

        tRes.tested("init()", result) ;
    }

    /**
    * Sets color model. <p>
    * Has <b> OK </b> status if no runtime exceptions occured
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
    * Has <b> OK </b> status if no runtime exceptions occured
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
    * Has <b> OK </b> status if no runtime exceptions occured
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
    * Has <b> OK </b> status if no runtime exceptions occured
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


