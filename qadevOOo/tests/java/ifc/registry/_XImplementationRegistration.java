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

package ifc.registry;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.registry.CannotRegisterImplementationException;
import com.sun.star.registry.XImplementationRegistration;
import com.sun.star.registry.XSimpleRegistry;
import com.sun.star.uno.RuntimeException;
import lib.MultiMethodTest;
import util.RegistryTools;
import util.utils;

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
* @see com.sun.star
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
        url = util.utils.getFullTestURL("qadevlibs/MyPersistObjectImpl.jar");
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
            if ("com.sun.star.cmp.MyPersistObject".
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
        requiredMethod("registerImplementation()") ;

        executeMethod("getImplementations()") ;
        executeMethod("checkInstantiation()") ;

        oObj.revokeImplementation(url, reg);
        RegistryTools.printRegistryInfo(reg, log) ;

        try {
            reg.getRootKey().getKeyNames();
        } catch (com.sun.star.registry.InvalidRegistryException e) {
            log.println("!!! Exception retrieving keys from registry :") ;
            e.printStackTrace(log);
        }

        tRes.tested("revokeImplementation()", true) ;
    }

    public void after() {
        this.disposeEnvironment() ;
    }
}


