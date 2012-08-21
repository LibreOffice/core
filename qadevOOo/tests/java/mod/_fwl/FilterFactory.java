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

import java.io.PrintWriter;
import java.util.ArrayList;

import lib.Status;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.utils;

import com.sun.star.beans.NamedValue;
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

        try {
            oInterface = ((XMultiServiceFactory)Param.getMSF()).createInstance
                ("com.sun.star.document.FilterFactory") ;
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't get service");
            e.printStackTrace(log);
            throw new StatusException("Couldn't get FilterFactory", e );
        }

        if (oInterface == null) {
            log.println("Service wasn't created") ;
            throw new StatusException(Status.failed("Service wasn't created")) ;
        }

        oObj = (XInterface) oInterface ;
        log.println("ImplName: "+utils.getImplName(oObj));

        log.println( "creating a new environment for object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        XNameAccess xNA = UnoRuntime.queryInterface
            (XNameAccess.class, oObj);
        String[] filterNames = xNA.getElementNames();

        // XNameContainer; XNameReplace
        String filterName = filterNames[0];
        Object[] instance = null;
        new PropertyValue();
        try{
            instance = (Object[]) xNA.getByName(filterName);
            getPropertyValue
                    (((PropertyValue[]) instance), "FilterService");
        } catch (com.sun.star.container.NoSuchElementException e){
            throw new StatusException(
            Status.failed("Couldn't get elements from object"));
        } catch (com.sun.star.lang.WrappedTargetException e){
            throw new StatusException(
            Status.failed("Couldn't get elements from object"));
        }

        log.println("adding INSTANCE 1 as obj relation to environment");

        log.println("fill property 'TemplateName' with 'INSTANCE1'");
        setPropertyValueValue((PropertyValue[])instance, "TemplateName", "INSTANCE1");
        tEnv.addObjRelation("INSTANCE" +1, instance);


        // XMSF
        ArrayList<String> vFTypes = new ArrayList<String>();
        ArrayList<Object[]> vFArgs = new ArrayList<Object[]>();
        for (int i = 0; i < filterNames.length; i++) {
            PropertyValue[] filterProps = null;
            try {
                System.out.println(filterNames[i]);
                filterProps = (PropertyValue[])
                    xNA.getByName(filterNames[i]);
            } catch (com.sun.star.lang.WrappedTargetException e) {
                throw new StatusException("Couldn't create relation", e);
            } catch (com.sun.star.container.NoSuchElementException e) {
                throw new StatusException("Couldn't create relation", e);
            }
            String filterImpl = (String) getPropertyValueValue
                (filterProps, "FilterService");
            if (filterImpl != null && filterImpl.length() > 0) {
                String filterType = (String) getPropertyValueValue
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

        // com.sun.star.container.XContainerQuery
        NamedValue[] querySequenze = new NamedValue[1];
        NamedValue query = new NamedValue();
        query.Name = "Name";
        query.Value = "Rich Text Format";
        querySequenze[0] = query;

        tEnv.addObjRelation("XContainerQuery.createSubSetEnumerationByProperties",
            querySequenze);


        return tEnv;
    } // finish method getTestEnvironment

    protected Object getPropertyValueValue(PropertyValue[] props, String pName) {
        int i = 0;
        while (i < props.length && !props[i].Name.equals(pName)) {
            i++;
        }
        return i < props.length ? props[i].Value : null;

    }

    protected void setPropertyValueValue(PropertyValue[] props, String pName, Object pValue) {
        int i = 0;
        while (i < props.length && !props[i].Name.equals(pName)) {
            i++;
        }
        props[i].Value = pValue;
    }

    protected PropertyValue getPropertyValue(PropertyValue[] props, String pName) {
        int i = 0;
        while (i < props.length && !props[i].Name.equals(pName)) {
            i++;
        }
        return i < props.length ? props[i] : null;

    }
}

