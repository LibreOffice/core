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

package ifc.ucb;

import lib.MultiMethodTest;

import com.sun.star.ucb.XDataContainer;

/**
* Testing <code>com.sun.star.ucb.XDataContainer</code>
* interface methods :
* <ul>
*  <li><code> getContentType()</code></li>
*  <li><code> setContentType()</code></li>
*  <li><code> getData()</code></li>
*  <li><code> setData()</code></li>
*  <li><code> getDataURL()</code></li>
*  <li><code> setDataURL()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.ucb.XDataContainer
*/
public class _XDataContainer extends MultiMethodTest {

    public static XDataContainer oObj = null;        // oObj filled by MultiMethodTest
    private final byte[] data = new byte[] {34, 35, 36} ;
    private String dataURL = null;

    /**
    * Sets the content type to some value. <p>
    * Has <b>OK</b> status if no runtime exseptions occurred.
    */
    public void _setContentType() {
        oObj.setContentType("image/jpeg") ;
        tRes.tested("setContentType()", true) ;
    }

    /**
    * Check if values 'set' and 'get' are equal. <p>
    * Has <b>OK</b> status if they are equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setContentType() </code>  </li>
    * </ul>
    */
    public void _getContentType() {
        requiredMethod("setContentType()") ;

        String type = oObj.getContentType() ;
        tRes.tested("getContentType()", "image/jpeg".equals(type)) ;
    }

    /**
    * Sets the data to some byte array. <p>
    * Has <b>OK</b> status if no runtime exseptions occurred.
    */
    public void _setData() {
        oObj.setData(data) ;
        tRes.tested("setData()", true) ;
    }

    /**
    * Check if arrays 'set' and 'get' are equal. <p>
    * Has <b>OK</b> status if they are equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setData() </code>  </li>
    * </ul>
    */
    public void _getData() {
        requiredMethod("setData()") ;

        byte[] gData = oObj.getData() ;
        boolean res = true ;
        if (res = (gData != null && gData.length == data.length)) {
            for (int i = 0; i < data.length; i++) {
                res &= data[i] == gData[i] ;
            }
        }

        tRes.tested("getData()", res) ;
    }

    /**
    * Sets the data URL to some URL. <p>
    * Has <b>OK</b> status if no runtime exseptions occurred.
    */
    public void _setDataURL() {
        dataURL = util.utils.getOfficeTemp(tParam.getMSF()) ;
        oObj.setDataURL(dataURL) ;
        tRes.tested("setDataURL()", true) ;
    }

    /**
    * Check if URLs 'set' and 'get' are equal. <p>
    * Has <b>OK</b> status if they are equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setDataURL() </code>  </li>
    * </ul>
    */
    public void _getDataURL() {
        requiredMethod("setDataURL()") ;

        String gURL = oObj.getDataURL() ;
        tRes.tested("getDataURL()", dataURL.equals(gURL)) ;
    }
}


