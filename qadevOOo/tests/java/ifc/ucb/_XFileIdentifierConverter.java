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
import com.sun.star.ucb.XFileIdentifierConverter;

/**
* Testing <code>com.sun.star.ucb.XFileIdentifierConverter</code>
* interface methods :
* <ul>
*  <li><code> getFileProviderLocality()</code></li>
*  <li><code> getFileURLFromSystemPath()</code></li>
*  <li><code> getSystemPathFromFileURL()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.ucb.XFileIdentifierConverter
*/
public class _XFileIdentifierConverter extends MultiMethodTest {

    /**
     * Contains the tested object.
     */
    public XFileIdentifierConverter oObj;

    /**
     * Gets the locality for SOffice temporary directory. <p>
     * Has <b> OK </b> status if the method returns value greater
     * than 0 (as office temp directory is supposed to be in the
     * same location). <p>
     */
    public void _getFileProviderLocality() {
        String baseURL = util.utils.getOfficeTemp((XMultiServiceFactory)tParam.getMSF());
        log.println("Using: "+baseURL);
        int loc = oObj.getFileProviderLocality(baseURL);
        log.println("Getting: "+loc);
        tRes.tested("getFileProviderLocality()",loc > 0);
    }

    /**
     * Tries to convert URL of SOffice temp directory to system
     * dependent path. <p>
     * Has <b> OK </b> status if the method returns system dependent
     * representation of the URL passed. <p>
     */
    public void _getSystemPathFromFileURL() {
        String baseURL = util.utils.getOfficeTemp((XMultiServiceFactory)tParam.getMSF());
        log.println("Using (Base): "+baseURL);
        String sysURL = util.utils.getOfficeTempDirSys((XMultiServiceFactory)tParam.getMSF());
        log.println("Using (System): "+sysURL);
        String get = oObj.getSystemPathFromFileURL(baseURL);
        log.println("Getting: "+get);
        //sysURL = sysURL.substring(0,sysURL.length()-1);
        tRes.tested("getSystemPathFromFileURL()",get.equals(sysURL));
    }

    /**
     * Tries to convert system dependent path of SOffice temp
     * directory to URL representation. <p>
     * Has <b> OK </b> status if the method returns URL representation
     * of the system dependent path passed. <p>
     */
    public void _getFileURLFromSystemPath() {
        String baseURL = util.utils.getOfficeTemp((XMultiServiceFactory)tParam.getMSF());
        log.println("Using (Base): "+baseURL);
        String sysURL = util.utils.getOfficeTempDirSys((XMultiServiceFactory)tParam.getMSF());
        log.println("Using (System): "+sysURL);
        String get = oObj.getFileURLFromSystemPath(sysURL,sysURL);
        log.println("Getting: "+get);
        tRes.tested("getFileURLFromSystemPath()",get.equals(baseURL));
    }

}
