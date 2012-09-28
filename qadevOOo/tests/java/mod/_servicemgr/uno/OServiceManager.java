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

package mod._servicemgr.uno;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XEnumeration;
import com.sun.star.container.XSet;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.XInterface;
import java.io.PrintWriter;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.comp.stoc.OServiceManager</code>. <p>
 *
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::lang::XMultiServiceFactory</code></li>
 *  <li> <code>com::sun::star::container::XContentEnumerationAccess</code></li>
 *  <li> <code>com::sun::star::container::XSet</code></li>
 *  <li> <code>com::sun::star::lang::XMultiComponentFactory</code></li>
 *  <li> <code>com::sun::star::lang::XServiceInfo</code></li>
 *  <li> <code>com::sun::star::container::XElementAccess</code></li>
 *  <li> <code>com::sun::star::container::XEnumerationAccess</code></li>
 *  <li> <code>com::sun::star::lang::XComponent</code></li>
 * </ul> <p>
 *
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 *
 * @see com.sun.star.lang.XMultiServiceFactory
 * @see com.sun.star.container.XContentEnumerationAccess
 * @see com.sun.star.container.XSet
 * @see com.sun.star.lang.XMultiComponentFactory
 * @see com.sun.star.lang.XServiceInfo
 * @see com.sun.star.container.XElementAccess
 * @see com.sun.star.container.XEnumerationAccess
 * @see com.sun.star.lang.XComponent
 * @see ifc.lang._XMultiServiceFactory
 * @see ifc.container._XContentEnumerationAccess
 * @see ifc.container._XSet
 * @see ifc.lang._XMultiComponentFactory
 * @see ifc.lang._XServiceInfo
 * @see ifc.container._XElementAccess
 * @see ifc.container._XEnumerationAccess
 * @see ifc.lang._XComponent
 */
public class OServiceManager extends TestCase {

    /**
     * Creating a Testenvironment for the interfaces to be tested.
     * Creates an instance of the service
     * <code>com.sun.star.comp.stoc.OServiceManager</code>.
     *     Object relations created :
     * <ul>
     *  <li> <code>'NewElement'</code> for
     *      {@link ifc.container._XSet} :
     *      element to be inserted into manager. Here
     *      a new <code>ServiceManager</code> implementation is
     *      create using
     *      <code>tools.SOConnect.createSimpleServiceManager()</code>
     *      and one of services contained in it is taken.</li>
     * </ul>
     */
    protected TestEnvironment createTestEnvironment
            (TestParameters Param, PrintWriter log) {

        XInterface oObj = null;
        Object oInterface = null;

        try {
            XMultiServiceFactory xMSF = (XMultiServiceFactory)Param.getMSF();
            oInterface = xMSF.createInstance
                ( "com.sun.star.comp.stoc.OServiceManager" );
        } catch( com.sun.star.uno.Exception e ) {
            log.println("ServiceManager service not available" );
        }

        // adding a service to the manager
        Object newElement = null ;
        XMultiServiceFactory srvMan = null;
        XComponentContext xContext = null;

        try {

            srvMan = com.sun.star.comp.helper.Bootstrap.createSimpleServiceManager();

        } catch ( java.lang.Exception ex) {
            log.println("Error creating SimpleServiceManager :");
            ex.printStackTrace(log);
        }

        try {
            XSet set = (XSet) UnoRuntime.queryInterface
                (XSet.class, oInterface) ;
            XSet set1 = (XSet) UnoRuntime.queryInterface
                (XSet.class, srvMan) ;
            XEnumeration oEnum = set1.createEnumeration();
            Object srv = oEnum.nextElement();

            set.insert(srv) ;

            newElement = oEnum.nextElement();

            XPropertySet xProp = (XPropertySet)UnoRuntime.queryInterface(
                                            XPropertySet.class, oInterface);
            if (xProp != null) {
                xContext = (XComponentContext) AnyConverter.toObject(
                    new Type(XComponentContext.class),
                            xProp.getPropertyValue("DefaultContext"));
            }
        } catch (com.sun.star.uno.Exception e) {
            log.println("Can't insert a service to the ServiceManager") ;
            throw new StatusException("Can't create object environment", e) ;
        }

        oObj = (XInterface) oInterface;

        log.println( "    creating a new environment for object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        // adding relation for XSet interface
        tEnv.addObjRelation("NewElement", newElement) ;

        // adding relation for XPropertySet
        tEnv.addObjRelation("PTT",new String[]{"DefaultContext","none","none"});

        //adding relation for XMultiComponentFactory
        if (xContext != null) {
            tEnv.addObjRelation("DC", xContext);
        }
        return tEnv;
    } // finish method getTestEnvironment

}    // finish class OServiceManager

