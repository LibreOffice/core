/*************************************************************************
 *
 *  $RCSfile: OServiceManager.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change:$Date: 2003-09-08 12:30:42 $
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
            XEnumeration enum = set1.createEnumeration();
            Object srv = enum.nextElement();

            set.insert(srv) ;

            newElement = enum.nextElement();

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

