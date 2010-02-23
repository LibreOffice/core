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

package ifc.loader;

import lib.MultiMethodTest;
import lib.StatusException;
import util.RegistryTools;

import com.sun.star.lang.XMultiServiceFactory;
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
 * Test is <b> Not </b> multithread compilant.
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

        String tmpDir = util.utils.getOfficeTempDir((XMultiServiceFactory)tParam.getMSF());

        try {
            xReg = RegistryTools.createRegistryService
                ((XMultiServiceFactory)tParam.getMSF()) ;

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

        if (rc == false)
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

        XServiceInfo xServInf = (XServiceInfo) UnoRuntime.queryInterface
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
    public void after() {
        this.disposeEnvironment() ;
    }
}

