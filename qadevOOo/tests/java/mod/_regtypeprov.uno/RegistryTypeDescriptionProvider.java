/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: RegistryTypeDescriptionProvider.java,v $
 * $Revision: 1.4 $
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

package mod._regtypeprov.uno;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XEnumeration;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import java.io.PrintWriter;
import lib.Status;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

/**
* Test for object which is represented by service
* <code>com.sun.star.reflection.TypeDescriptionProvider</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::container::XHierarchicalNameAccess</code></li>
*  <li> <code>com::sun::star::lang::XComponent</code></li>
* </ul>
* This object test <b> is NOT </b> designed to be run in several
* threads concurently.
* @see com.sun.star.container.XHierarchicalNameAccess
* @see com.sun.star.lang.XComponent
* @see com.sun.star.reflection.TypeDescriptionProvider
* @see ifc.container._XHierarchicalNameAccess
* @see ifc.lang._XComponent
*/
public class RegistryTypeDescriptionProvider extends TestCase {

    protected void initialize ( TestParameters Param, PrintWriter log) {

    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.reflection.TypeDescriptionProvider</code>.
    *     Object relations created :
    * <ul>
    *  <li> <code>'ElementName'</code> for
    *      {@link ifc.container._XHierarchicalNameAccess} :
    *      name of the existing element. Here it type name for
    *      <code>com.sun.star.uno.XInterface</code> interface.</li>
    * </ul>
    */
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {
        XInterface oObj = null;
        Object oInterface = null;

        try {
            XMultiServiceFactory xMSF = (XMultiServiceFactory)Param.getMSF();
            XPropertySet xProp = (XPropertySet)UnoRuntime.queryInterface(
                                                XPropertySet.class, xMSF);
            // get context
            XComponentContext xContext = (XComponentContext)
                            UnoRuntime.queryInterface(XComponentContext.class,
                            xProp.getPropertyValue("DefaultContext"));
            // get the type description manager
            Object o = xContext.getValueByName("/singletons/" +
                        "com.sun.star.reflection.theTypeDescriptionManager");
            // the manager contains all providers
            XEnumerationAccess aProviderAccess = (XEnumerationAccess)
                        UnoRuntime.queryInterface(XEnumerationAccess.class, o);
            // collect enumeration
            XEnumeration xProviderEnum;
            if (aProviderAccess.hasElements())
                xProviderEnum = aProviderAccess.createEnumeration();
            else
                throw new lib.StatusException(Status.failed(
                                        "No TypeDescriptionProvider given"));
            // take the first registry type description provider
            oInterface = xProviderEnum.nextElement();
            oObj = (XInterface) AnyConverter.toObject(
                            new Type(XInterface.class),oInterface);
        }
        catch( com.sun.star.uno.Exception e ) {
            log.println("Service not available" );
        }

        //oObj = (XInterface) oInterface;

        log.println( "    creating a new environment for object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        // adding relation for XHierarchicalNameAccess
        tEnv.addObjRelation("ElementName", "com.sun.star.uno.XInterface") ;
        tEnv.addObjRelation("SearchString", "com.sun.star.loader");

        return tEnv;
    } // finish method getTestEnvironment

}

