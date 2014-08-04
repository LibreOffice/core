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

import com.sun.star.ucb.XFileIdentifierConverter;

/**
* Testing <code>com.sun.star.ucb.XFileIdentifierConverter</code>
* interface methods :
* <ul>
*  <li><code> getFileProviderLocality()</code></li>
*  <li><code> getFileURLFromSystemPath()</code></li>
*  <li><code> getSystemPathFromFileURL()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
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
        String baseURL = util.utils.getOfficeTemp(tParam.getMSF());
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
        String baseURL = util.utils.getOfficeTemp(tParam.getMSF());
        log.println("Using (Base): "+baseURL);
        String sysURL = util.utils.getOfficeTempDirSys(tParam.getMSF());
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
        String baseURL = util.utils.getOfficeTemp(tParam.getMSF());
        log.println("Using (Base): "+baseURL);
        String sysURL = util.utils.getOfficeTempDirSys(tParam.getMSF());
        log.println("Using (System): "+sysURL);
        String get = oObj.getFileURLFromSystemPath(sysURL,sysURL);
        log.println("Getting: "+get);
        tRes.tested("getFileURLFromSystemPath()",get.equals(baseURL));
    }

}
