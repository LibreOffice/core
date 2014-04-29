/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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


