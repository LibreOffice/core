/*************************************************************************
 *
 *  $RCSfile: TypeDetection.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change:$Date: 2003-11-18 16:28:53 $
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

package mod._fwl;

import com.sun.star.container.XNameAccess;
import java.io.PrintWriter;

import lib.Status;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.utils;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.beans.PropertyValue;
import com.sun.star.uno.AnyConverter;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.document.TypeDetection</code>. <p>
 *
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::container::XContainerQuery</code></li>
 *  <li> <code>com::sun::star::container::XElementAccess</code></li>
 *  <li> <code>com::sun::star::container::XNameAccess</code></li>
 *  <li> <code>com::sun::star::container::XNameContainer</code></li>
 *  <li> <code>com::sun::star::container::XNameReplace</code></li>
 *  <li> <code>com::sun::star::document::XTypeDetection</code></li>
 *  <li> <code>com::sun::star::util::XFlushable</code></li>
 * </ul> <p>
 *
 * @see com.sun.star.container.XContainerQuery
 * @see com.sun.star.container.XElementAccess
 * @see com.sun.star.container.XNameAccess
 * @see com.sun.star.container.XNameContainer
 * @see com.sun.star.container.XNameReplace
 * @see com.sun.star.document.XTypeDetection
 * @see com.sun.star.util.XFlushable
 * @see ifc.container._XContainerQuery
 * @see ifc.container._XElementAccess
 * @see ifc.container._XNameAccess
 * @see ifc.container._XNameContainer
 * @see ifc.container._XNameReplace
 * @see ifc.document._XTypeDetection
 * @see ifc.util.XFlushable
 */
public class TypeDetection extends TestCase {

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.document.TypeDetection</code>. <p>
    */
    protected TestEnvironment createTestEnvironment
            (TestParameters Param, PrintWriter log) {
        XInterface oObj = null;
        Object oInterface = null ;

        try {
            oInterface = ((XMultiServiceFactory)Param.getMSF()).createInstance
                ("com.sun.star.document.TypeDetection") ;
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't get service");
            e.printStackTrace(log);
            throw new StatusException("Couldn't get TypeDetection", e );
        }

        if (oInterface == null) {
            log.println("Service wasn't created") ;
            throw new StatusException(Status.failed("Service wasn't created")) ;
        }

        oObj = (XInterface) oInterface ;
        log.println("ImplName: "+utils.getImplName(oObj));

        log.println( "creating a new environment for object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        XNameAccess xNA = (XNameAccess) UnoRuntime.queryInterface
            (XNameAccess.class, oObj);
        String[] elementNames = xNA.getElementNames();
        String elementName = elementNames[0];
        //PropertyValue instance = new PropertyValue();
        Object[] instance = null;;
        PropertyValue instanceProp = new PropertyValue();
        try{
            instance = (Object[]) xNA.getByName(elementName);
            instanceProp = (PropertyValue) instance[6];
        } catch (com.sun.star.container.NoSuchElementException e){
            throw new StatusException(
            Status.failed("Couldn't get elements from object"));
        } catch (com.sun.star.lang.WrappedTargetException e){
            throw new StatusException(
            Status.failed("Couldn't get elements from object"));
        }

        log.println("adding INSTANCEn as obj relation to environment");

        int THRCNT = Integer.parseInt((String) Param.get("THRCNT"));

        for (int n = 1; n < (THRCNT + 1); n++) {
            log.println("adding INSTANCE" + n +
                        " as obj relation to environment");

            instanceProp.Value = "INSTANCE"+ n + System.currentTimeMillis();
            instance[6] = instanceProp;

            tEnv.addObjRelation("INSTANCE" + n, instance);
        }


        return tEnv;
    } // finish method getTestEnvironment

}

