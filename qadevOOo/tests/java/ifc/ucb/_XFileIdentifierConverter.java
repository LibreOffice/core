/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XFileIdentifierConverter.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:32:01 $
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
