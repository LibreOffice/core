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

package ifc.loader;

import lib.MultiMethodTest;
import lib.StatusException;
import util.RegistryTools;

import com.sun.star.lang.XServiceInfo;
import com.sun.star.loader.CannotActivateFactoryException;
import com.sun.star.loader.XImplementationLoader;
import com.sun.star.registry.CannotRegisterImplementationException;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.registry.XSimpleRegistry;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Testing <code>com.sun.star.loader.XImplementationLoader</code>
 * interface methods :
 * <ul>
 *  <li><code> activate()</code></li>
 *  <li><code> writeRegistryInfo()</code></li>
 * </ul> <p>
 *
 * The following object relations required :
 * <ul>
 *  <li> <code>'ImplementationLoader'</code> : service which is
 *    responsible for loading implementations. </li>
 *  <li> <code>'ImplementationUrl'</code> : implementation file location. </li>
 *  <li> <code>'ImplementationName'</code> : Name of the implementation.</li>
 * </ul> <p>
 * Object has to be recreated after this test. <p>
 * Test is <b> Not </b> multithread compliant.
 */
public class _XImplementationLoader extends MultiMethodTest {

    public XImplementationLoader oObj = null;
    private String implLoader = null ;
    private String implUrl = null ;
    private String implName = null ;

    /**
    * Retrieves object relations.
    * @throws StatusException If one of relations not found.
    */
    @Override
    public void before() {
        implLoader = (String) tEnv.getObjRelation("ImplementationLoader") ;
        implUrl = (String) tEnv.getObjRelation("ImplementationUrl") ;
        implName = (String) tEnv.getObjRelation("ImplementationName") ;

        if (implLoader == null || implUrl == null || implName == null)
            throw new StatusException("One of object relations not found",
                new NullPointerException()) ;
    }

    /**
    * First registry file created, and the root key retrieved.
    * Then method <code>writeRegistryInfo</code> called and it must
    * write some info into the registry root key. After all registry
    * is destroyed.<p>
    * Has OK status if some info was written into registry.
    */
    public void _writeRegistryInfo() {
        XRegistryKey key ;
        XSimpleRegistry xReg = null ;

        String tmpDir = util.utils.getOfficeTempDir(tParam.getMSF());

        try {
            xReg = RegistryTools.createRegistryService
                (tParam.getMSF()) ;

            xReg.open(tmpDir + "XImpLoader_tmp.rdb", false, true) ;

            key = xReg.getRootKey() ;
        } catch (com.sun.star.uno.Exception e) {
            log.println("Can not create registry for writing") ;
            e.printStackTrace(log) ;
            tRes.tested("writeRegistryInfo()", false) ;
            return ;
        }

        boolean rc ;
        try {
            rc = oObj.writeRegistryInfo(key, implLoader, implUrl) ;
        } catch (CannotRegisterImplementationException e) {
            throw new StatusException("Can not register implementation", e) ;
        }

        if (!rc)
            log.println("Method returned false value") ;

        String[] keys ;
        try {
            keys = key.getKeyNames() ;
        } catch (com.sun.star.uno.Exception e) {
            log.println("Error retrieving key names from registry") ;
            tRes.tested("writeRegistryInfo()", false) ;
            return ;
        }

        // destroying registry file
        try {
            xReg.close() ;
            xReg.destroy() ;
        } catch (com.sun.star.registry.InvalidRegistryException e) {
            log.println("Can't destroy registry file.") ;
        }

        tRes.tested("writeRegistryInfo()", rc && keys.length > 0) ;
    }

    /**
    * Tries to activate the implementation. <p>
    *
    * Has OK status if not <code>null</code> value returned by method,
    * if its implementation name is the same as expected.
    */
    public void _activate() {
        boolean ok = true ;
        XInterface factory = null ;

        try {
            factory = (XInterface) oObj.activate
                (implName, implLoader, implUrl, null) ;
        } catch (CannotActivateFactoryException e) {
            throw new StatusException("Can not activate factory", e) ;
        }

        XServiceInfo xServInf = UnoRuntime.queryInterface
            (XServiceInfo.class, factory) ;

        if (xServInf == null) {
            if (factory == null) {
                log.println("activate() returns null - FAILED.");
            } else {
                log.println("Activated impementation doesn't support "+
                    "XServiceInfo - FAILED.");
            }
            ok = false ;
        } else {
            String gImpName = xServInf.getImplementationName() ;
            log.println("Implementation name returned :" + gImpName);

            if (!gImpName.equals(implName)) {
                log.println("!!! But other name was expected :" + implName);
                ok = false ;
            }
        }

        tRes.tested("activate()", ok) ;
    }

    /**
    * Forces object recreation.
    */
    @Override
    public void after() {
        this.disposeEnvironment() ;
    }
}

