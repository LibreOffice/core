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

package mod._smgr;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XEnumeration;
import com.sun.star.container.XSet;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.XInterface;

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

