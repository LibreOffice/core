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

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.utils;

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
    @Override
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
     * Creating a TestEnvironment for the interfaces to be tested.
     * Creates an instance of the service
     * <code>com.sun.star.comp.framework.PathSettings</code>.
     */
    @Override
    protected TestEnvironment createTestEnvironment
            (TestParameters Param, PrintWriter log) throws Exception {
        XInterface oObj = null;
        //now get the OButtonControl
        Object oInterface = Param.getMSF().createInstance
            ("com.sun.star.comp.framework.PathSettings") ;

        oObj = (XInterface) oInterface ;
        log.println("ImplName: "+utils.getImplName(oObj));

        log.println( "creating a new environment for object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        Set<String> exclProps = new HashSet<String>();
        exclProps.add("UIConfig");
        tEnv.addObjRelation("XFastPropertySet.ExcludeProps", exclProps);
        tEnv.addObjRelation("XMultiPropertySet.ExcludeProps", exclProps);

        saveAllPropertyValues(oObj);

        return tEnv;
    } // finish method getTestEnvironment

    private void saveAllPropertyValues(XInterface oObj){

        xPS = UnoRuntime.queryInterface(
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

