/*************************************************************************
 *
 *  $RCSfile: RegistryTypeDescriptionProvider.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change:$Date: 2003-09-08 12:00:36 $
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

package mod._rdbtdp;

import java.io.PrintWriter;

import lib.Status;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XEnumeration;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.XInterface;

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
    public TestEnvironment createTestEnvironment( TestParameters Param,
                                                  PrintWriter log )
                                                    throws StatusException {
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

