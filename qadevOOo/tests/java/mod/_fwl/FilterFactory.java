/*************************************************************************
 *
 *  $RCSfile: FilterFactory.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 11:54:01 $
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

import java.io.PrintWriter;
import java.util.Vector;

import lib.Status;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.utils;

import com.sun.star.beans.PropertyValue;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.comp.framework.FilterFactory</code>. <p>
 *
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::container::XNameAccess</code></li>
 *  <li> <code>com::sun::star::container::XElementAccess</code></li>
 *  <li> <code>com::sun::star::lang::XMultiServiceFactory</code></li>
 * </ul> <p>
 *
 * @see com.sun.star.container.XNameAccess
 * @see com.sun.star.container.XElementAccess
 * @see com.sun.star.lang.XMultiServiceFactory
 * @see ifc.container._XNameAccess
 * @see ifc.container._XElementAccess
 * @see ifc.lang._XMultiServiceFactory
 */
public class FilterFactory extends TestCase {

    /**
     * Creating a Testenvironment for the interfaces to be tested.
     * Creates an instance of the service
     * <code>com.sun.star.comp.framework.FilterFactory</code>.
     * Prepares relation for
     * <code>XMultiServiceFactory.createInstanceWithArguments</code> method
     * test. Only filters wich have their service names can be instantiated
     * in some reasons (?). For instantiation used filter type as service
     * name and its name as a parameter. <p>
     */
    protected TestEnvironment createTestEnvironment
            (TestParameters Param, PrintWriter log) {
        XInterface oObj = null;
        Object oInterface = null ;

        //now get the OButtonControl
        try {
            oInterface = ((XMultiServiceFactory)Param.getMSF()).createInstance
                ("com.sun.star.comp.framework.FilterFactory") ;
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't get service");
            e.printStackTrace(log);
            throw new StatusException("Couldn't get GridControl", e );
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
        String[] filterNames = xNA.getElementNames();
        Vector vFTypes = new Vector();
        Vector vFArgs = new Vector();
        for (int i = 0; i < filterNames.length; i++) {
            PropertyValue[] filterProps = null;
            try {
                filterProps = (PropertyValue[])
                    xNA.getByName(filterNames[i]);
            } catch (com.sun.star.lang.WrappedTargetException e) {
                throw new StatusException("Couldn't create relation", e);
            } catch (com.sun.star.container.NoSuchElementException e) {
                throw new StatusException("Couldn't create relation", e);
            }

            String filterImpl = (String) getPropertyValue
                (filterProps, "FilterService");
            if (filterImpl != null && filterImpl.length() > 0) {
                String filterType = (String) getPropertyValue
                    (filterProps, "Type");
                vFTypes.add(filterType);
                PropertyValue prop = new PropertyValue();
                prop.Name = "FilterName";
                prop.Value = filterNames[i];
                vFArgs.add(new Object[] {prop});

                log.println("Found filter '" + filterNames[i] + "', type: " +
                    filterType + ", service name: '" + filterImpl + "'");
            }
        }

        tEnv.addObjRelation("XMSF.serviceNamesWithArgs",
            vFTypes.toArray(new String[vFTypes.size()]));
        tEnv.addObjRelation("XMSF.Args",
            vFArgs.toArray(new Object[vFArgs.size()][]));

        return tEnv;
    } // finish method getTestEnvironment

    protected Object getPropertyValue(PropertyValue[] props, String pName) {
        int i = 0;
        while (i < props.length && !props[i].Name.equals(pName)) i++;
        return i < props.length ? props[i].Value : null;
    }
}

