/*************************************************************************
 *
 *  $RCSfile: _XImplementationLoader.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 10:48:07 $
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
        tEnv.dispose() ;
    }
}

