/*************************************************************************
 *
 *  $RCSfile: _XDataContainer.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 11:24:34 $
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
    * Has <b>OK</b> status if no runtime exseptions occured.
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
    * Has <b>OK</b> status if no runtime exseptions occured.
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
    * Has <b>OK</b> status if no runtime exseptions occured.
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


