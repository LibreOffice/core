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

package ifc.ucb;

import lib.MultiMethodTest;

import com.sun.star.lang.XMultiServiceFactory;
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
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.ucb.XDataContainer
*/
public class _XDataContainer extends MultiMethodTest {

    public static XDataContainer oObj = null;        // oObj filled by MultiMethodTest
    private byte[] data = new byte[] {34, 35, 36} ;
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
        dataURL = util.utils.getOfficeTemp((XMultiServiceFactory)tParam.getMSF()) ;
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


