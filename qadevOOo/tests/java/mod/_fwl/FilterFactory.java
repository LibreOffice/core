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

        XNameAccess xNA = (XNameAccess) UnoRuntime.queryInterface
            (XNameAccess.class, oObj);
        String[] filterNames = xNA.getElementNames();

        // XNameContainer; XNameReplace
        String filterName = filterNames[0];
        Object[] instance = null;;
        PropertyValue instanceProp = new PropertyValue();
        try{
            instance = (Object[]) xNA.getByName(filterName);
            PropertyValue[] props = (PropertyValue[]) instance;
            instanceProp = (PropertyValue) getPropertyValue
                    (((PropertyValue[]) instance), "FilterService"); //instance[9];
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
        Vector vFTypes = new Vector();
        Vector vFArgs = new Vector();
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

