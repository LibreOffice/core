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

package mod._fwl;

import com.sun.star.beans.NamedValue;
import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyVetoException;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.uno.UnoRuntime;
import java.io.PrintWriter;
import java.util.HashSet;
import java.util.Set;

import lib.Status;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.utils;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.comp.framework.PathSettings</code>. <p>
 *
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::beans::XFastPropertySet</code></li>
 *  <li> <code>com::sun::star::util::PathSettings</code></li>
 *  <li> <code>com::sun::star::beans::XPropertySet</code></li>
 *  <li> <code>com::sun::star::beans::XMultiPropertySet</code></li>
 * </ul> <p>
 *
 * @see com.sun.star.beans.XFastPropertySet
 * @see com.sun.star.util.PathSettings
 * @see com.sun.star.beans.XPropertySet
 * @see com.sun.star.beans.XMultiPropertySet
 * @see ifc.beans._XFastPropertySet
 * @see ifc.util._PathSettings
 * @see ifc.beans._XPropertySet
 * @see ifc.beans._XMultiPropertySet
 */
public class PathSettings extends TestCase {

    private static NamedValue[]  m_Properties;
    private static XPropertySet xPS;

    /**
     * restores the old values of the path settings
     * @param tParam the test parameter
     * @param log the log writer
     */
    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("restore old values of path settings...");

        for (int i=0; i < m_Properties.length; i++){
            try{

                xPS.setPropertyValue(m_Properties[i].Name, m_Properties[i].Value);

            } catch (com.sun.star.beans.UnknownPropertyException e){
            } catch (PropertyVetoException e){
            } catch (IllegalArgumentException e){
            } catch (WrappedTargetException e){
            }
        }
    }
    /**
     * Creating a Testenvironment for the interfaces to be tested.
     * Creates an instance of the service
     * <code>com.sun.star.comp.framework.PathSettings</code>.
     */
    protected TestEnvironment createTestEnvironment
            (TestParameters Param, PrintWriter log) {
        XInterface oObj = null;
        Object oInterface = null ;

        //now get the OButtonControl
        try {
            oInterface = ((XMultiServiceFactory)Param.getMSF()).createInstance
                ("com.sun.star.comp.framework.PathSettings") ;
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

        Set exclProps = new HashSet();
        exclProps.add("UIConfig");
        tEnv.addObjRelation("XFastPropertySet.ExcludeProps", exclProps);
        tEnv.addObjRelation("XMultiPropertySet.ExcludeProps", exclProps);

        saveAllPropertyValues(oObj);

        return tEnv;
    } // finish method getTestEnvironment

    private void saveAllPropertyValues(XInterface oObj){

        xPS = (XPropertySet) UnoRuntime.queryInterface(
                                                XPropertySet.class, oObj);

        XPropertySetInfo xPSI = xPS.getPropertySetInfo();

        Property[] allProperties = xPSI.getProperties();
        m_Properties = new NamedValue[allProperties.length];

        for (int i=0; i < allProperties.length; i++){
            try{
                m_Properties[i] = new NamedValue(allProperties[i].Name,
                                   xPS.getPropertyValue(allProperties[i].Name));

            } catch (com.sun.star.beans.UnknownPropertyException e){
            } catch (WrappedTargetException e){
            }
        }
    }

}

