/*************************************************************************
 *
 *  $RCSfile: _XImplementationRegistration.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 10:49:34 $
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

package ifc.registry;

import lib.MultiMethodTest;
import util.RegistryTools;
import util.utils;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.registry.CannotRegisterImplementationException;
import com.sun.star.registry.XImplementationRegistration;
import com.sun.star.registry.XSimpleRegistry;
import com.sun.star.uno.RuntimeException;

/**
* Testing <code>com.sun.star.registry.XImplementationRegistration</code>
* interface methods :
* <ul>
*  <li><code> registerImplementation()</code></li>
*  <li><code> revokeImplementation()</code></li>
*  <li><code> getImplementations()</code></li>
*  <li><code> checkInstantiation()</code></li>
* </ul> <p>
* The following predefined files needed to complete the test:
* <ul>
*  <li> <code>solibrary.jar</code> : jar file with implementation
*   classes. One of the required implementation must have name
*   <code>com.ivistaportal.solibrary.HistogramImpl</code> cause
*   it is checked in <code>getImplementations</code> method. </li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* After test completion object environment has to be recreated.
* @see com.sun.star.###
*/
public class _XImplementationRegistration extends MultiMethodTest {

    public XImplementationRegistration oObj = null;

    private String url = null ;
    private String loader = null ;
    private XSimpleRegistry reg = null ;

    /**
    * First a registry created and opened in the temporary directory
    * of StarOffice. Then some implementations situated in JAR file
    * is registered in the registry opened. <p>
    * Has <b>OK</b> status if some information is written into registry.
    *
    */
    public void _registerImplementation()
        throws CannotRegisterImplementationException, RuntimeException
    {
/*        url = tParam.get("DOCPTH") +
            System.getProperty("file.separator") + "XImpReg.dll" ;
        loader = "com.sun.star.loader.SharedLibrary" ;*/
        url = util.utils.getFullTestURL("solibrary.jar");
        loader = "com.sun.star.loader.Java2";
        boolean result = false ;
        String name = null;

        try {
            name = utils.getOfficeTempDir((XMultiServiceFactory)tParam.getMSF()) +
                "XImplementationRegistration_tmp.rdb";
            reg = RegistryTools.openRegistry
                (name, (XMultiServiceFactory)tParam.getMSF()) ;

            oObj.registerImplementation(loader, url, reg) ;

            RegistryTools.printRegistryInfo(reg.getRootKey(), log) ;

            String[] subKeys = reg.getRootKey().getKeyNames() ;

            result = subKeys != null && subKeys.length > 0 ;

        } catch (com.sun.star.uno.Exception e) {
            log.println("Can't open registry file: " + name) ;
            e.printStackTrace(log) ;
        }
        tRes.tested("registerImplementation()", result) ;
    }

    /**
    * Retrieves an array of implementation names and check them. <p>
    * Has <b>OK</b> status if among them an implementation name
    * <code>com.ivistaportal.solibrary.HistogramImpl</code> exists.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> registerImplementation </code>  </li>
    * </ul>
    */
    public void _getImplementations() throws RuntimeException{
        requiredMethod("registerImplementation()") ;

        String[] impl = oObj.getImplementations(loader, url) ;

        if (impl.length == 0) log.println("getImplementations() "+
            "returns an empty array");
        boolean result = false ;
        log.println("Implementations found :") ;
        for (int i = 0; i < impl.length; i++) {
            log.println(" '" + impl[i] + "'") ;
            if ("com.ivistaportal.solibrary.HistogramImpl".
                equals(impl[i])) {

                result = true ;
                break ;
            }
        }



        tRes.tested("getImplementations()", result) ;
    }

    /**
    * Calls the method with
    * <code>com.sun.star.comp.stoc.JavaComponentLoader</code>
    * implementation name.<p>
    * Has <b>OK</b> status if not null array returned. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> registerImplementation </code>  </li>
    * </ul>
    */
    public void _checkInstantiation() throws RuntimeException {
        requiredMethod("registerImplementation()") ;

        String[] inst = oObj.checkInstantiation(
            "com.sun.star.comp.stoc.JavaComponentLoader") ;

        tRes.tested("checkInstantiation()", inst != null) ;
    }

    /**
    * Revokes implementations from registry, and checks if
    * all implementations' information is deleted. <p>
    * Has <b>OK</b> status if registry has no key entries. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> registerImplementation </code> : to have
    *   implementation registered in registry. </li>
    * </ul>
    * The following method tests are to be executed before :
    * <ul>
    *  <li> <code> getImplementations </code>
    *  <li> <code> checkInstantiation </code>
    * </ul>
    */
    public void _revokeImplementation() throws RuntimeException{
        boolean result = false ;

        requiredMethod("registerImplementation()") ;

        executeMethod("getImplementations()") ;
        executeMethod("checkInstantiation()") ;

        oObj.revokeImplementation(url, reg);
        RegistryTools.printRegistryInfo(reg, log) ;

        try {
            String[] subKeys = reg.getRootKey().getKeyNames() ;

            result = subKeys == null || subKeys.length == 0 ;
        } catch (com.sun.star.registry.InvalidRegistryException e) {
            log.println("!!! Exception retrieving keys from registry :") ;
            e.printStackTrace(log);
        }

        tRes.tested("revokeImplementation()", result) ;
    }

    public void after() {
        tEnv.dispose() ;
    }
}


