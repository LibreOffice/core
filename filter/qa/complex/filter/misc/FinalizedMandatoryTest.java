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
package complex.filter.misc;

import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;

import com.sun.star.beans.PropertyValue;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameContainer;
import com.sun.star.container.XNameReplace;
import com.sun.star.lang.WrappedTargetRuntimeException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XFlushable;


/**
 * This complex test checks the functionality of the properties "Finalized" and "Mandatory" of
 * the services <CODE>com.sun.star.document.FilterFactory</CODE> and
 * <CODE>com.sun.star.document.TypeDetection</CODE>.
 *
 * Each of theses services represent a container of <CODE>PropertyValue[]</CODE>.
 * The <CODE>PropertyValue[]</CODE> contains among others the properties called
 * <CODE>Finalized</CODE> and <CODE>Mandatory</CODE>. If the property
 * <CODE>Finalized</CODE> is set to <CODE>true</CODE>, a filter can be removed
 * but will not be able to be changed.
 * If the property <CODE>Mandatory</CODE> is set to <CODE>true</CODE>, the filter
 * can not be removed.
 *
 * Every filter, which is registered to the office, will be tested. For every filter-test
 * a new instance of the mentioned services will be created.

 * During the test the property <CODE>UIName</CODE>
 * will be changed and the service will be flushed. The test checks for expected exceptions:
 * If the property <CODE>Finalized</CODE> equals
 * <CODE>true</CODE> the tests check if an <CODE>Exception</CODE> must be thrown.
 * The next step of the test is the removal of the filter was removed, than the service
 * will be flushed. The test checks for expected exceptions: If the property
 * <CODE>Madantory</CODE> equals <CODE>true</CODE>, an <CODE>Exception</CODE> must
 * be thrown.
 * This test results <CODE>false</CODE> state if there is no filter available with:
 * <CODE>Finalized=true</CODE>
 * <CODE>Finalized=false</CODE>
 * <CODE>Mandatory=true</CODE>
 * <CODE>Mandatory=false</CODE>
 */
public class FinalizedMandatoryTest
{

    XMultiServiceFactory xMSF;

    /** Create the environment for following tests.
     * Use either a component loader from desktop or
     * from frame
     * @throws Exception Exception
     */
    @Before public void before() throws Exception
    {

        // create TypeDetection
        xMSF = getMSF();
        assertNotNull("Could not get XMultiServiceFactory", xMSF);

    }

    /**
     * Creates an instance for the given <CODE>serviceName</CODE>
     * @param serviceName the name of the service which should be created
     * @throws Exception was thrown if creation fails
     * @return <CODE>XInterface</CODE> of service
     */
    private XInterface getTestObject(String serviceName) throws Exception
    {

        Object oInterface = xMSF.createInstance(serviceName);

        assertNotNull("Service wan't created", oInterface);
        return (XInterface) oInterface;
    }

    /**
     * call the function <CODE>checkReadonlySupport</CODE> to test <CODE>com.sun.star.document.FilterFactory</CODE>
     * @see com.sun.star.document.FilterFactory
     */
    @Test public void checkReadonlySupportFilterFactory() throws Exception
    {
        checkReadonlySupport("com.sun.star.document.FilterFactory");
    }

    /**
     * call the function <CODE>checkReadonlySupport</CODE> to test <CODE>com.sun.star.document.TypeDetection</CODE>
     * @see com.sun.star.document.TypeDetection
     */
    @Test public void checkReadonlySupportTypeDetection() throws Exception
    {
        checkReadonlySupport("com.sun.star.document.TypeDetection");
    }

    /**
     * test the given service <CODE>serviceName</CODE>.
     * For every filter a new instance was created and the tests started.
     * @param serviceName the name of the service to test
     */
    private void checkReadonlySupport(String serviceName) throws Exception
    {
        System.out.println("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
        System.out.println("testing service '" + serviceName + "'");

        XInterface oObj = null;
        oObj = getTestObject(serviceName);
        System.out.println("ImplName: " + util.utils.getImplName(oObj));

        boolean mandantoryTrue = false;
        boolean mandantoryFalse = false;
        boolean finalizedTrue = false;
        boolean finalizedFalse = false;


        XNameAccess xNA = UnoRuntime.queryInterface(XNameAccess.class, oObj);
        String[] filterNames = xNA.getElementNames();

        // XNameContainer; XNameReplace
        String filterName = filterNames[0];
        Object[] instance = null;

        for (int i = 0; i < filterNames.length; i++)
        {
            System.out.println("------------------------------------------------");
            filterName = filterNames[i];
            System.out.println(filterName);

            // testobject must new created for every test.
            // We change in a loop the container and try to flush this changes.
            // If we get an expected exception this container is corrupt. It's
            // similar to a document which could not be saved because of invalid
            // contend. While you don't remove the invalid conted you will never
            // be able to save the document. Same here.
            oObj = getTestObject(serviceName);

            xNA = UnoRuntime.queryInterface(XNameAccess.class, oObj);
            XNameContainer xNC = UnoRuntime.queryInterface(XNameContainer.class, oObj);
            XNameReplace xNR = UnoRuntime.queryInterface(XNameReplace.class, oObj);
            XFlushable xFlush = UnoRuntime.queryInterface(XFlushable.class, oObj);

            instance = (Object[]) xNA.getByName(filterName);
            PropertyValue[] props = (PropertyValue[]) instance;

            printPropertyValues(props);

            boolean isMandatory = ((Boolean) getPropertyValueValue(props, "Mandatory")).booleanValue();
            boolean isFinalized = ((Boolean) getPropertyValueValue(props, "Finalized")).booleanValue();

            // memory if every state is available
            mandantoryTrue |= isMandatory;
            mandantoryFalse |= !isMandatory;

            finalizedTrue |= isFinalized;
            finalizedFalse |= !isFinalized;

            //change the filter
            setPropertyValueValue((PropertyValue[]) instance, "UIName", "dummy");

            // 1a.) try to change the filter in the container
            xNR.replaceByName(filterName, instance);

            // 1b.) try to wirte the changed filter to the configuration.
            // This must result in a exception if the filter is finalized.
            boolean flushError = false;
            try
            {
                xFlush.flush();
            }
            catch (WrappedTargetRuntimeException e)
            {
                flushError = true;
                assertTrue("Unexpected exception wihle flushing changed filter '" + filterName + "'", isFinalized);
            }
            assertTrue("Expected exception was not thorwn while flushing changed filter '" + filterName + "' Finalized:" + isFinalized,
                    !(flushError ^ isFinalized));



            // 2a.) try to remove the filter from the container
            xNC.removeByName(filterName);
            // 1b.) try to wirte the changed filter to the configuration.
            // This must result in a exception if the filter is mandatory
            flushError = false;
            try
            {
                xFlush.flush();
            }
            catch (WrappedTargetRuntimeException e)
            {
                flushError = true;
                assertTrue("Unexpected exception wihle flushing removed filter '" + filterName + "'", isMandatory);
            }
            assertTrue("Expected exception was not thorwn while flushing removed filter '" + filterName + "' Mandatory:" + isMandatory,
                    !(flushError ^ isMandatory));
        }
        String preMsg = "Could not find filter with state ";
        String postMsg = " Please check if such filter is installed!";
        assertTrue(preMsg + "'Mandatory=true'" + postMsg, mandantoryTrue);
        assertTrue(preMsg + "'Mandatory=false'" + postMsg, mandantoryFalse);
        assertTrue(preMsg + "'Finalized=true'" + postMsg, finalizedTrue);
        assertTrue(preMsg + "'Finalized=false'" + postMsg, finalizedFalse);
    }

    /**
     * print all properties with its values to <CODE>logger</CODE>. For debug purposes.
     * @see stats.SimpleLogWriter
     * @see com.sun.star.beans.PropertyValue
     * @param props Sequenze of PropertyValue
     */
    protected void printPropertyValues(PropertyValue[] props)
    {
        int i = 0;
        while (i < props.length)
        {
            System.out.println(props[i].Name + ":" + props[i].Value.toString());
            i++;
        }
        if (i < props.length)
        {
            System.out.println(props[i].Name + ":" + props[i].Value.toString());
        }
    }

    /**
     * returns the value of the specified (<CODE>pName</CODE>) property from a sequenze of <CODE>PropertyValue</CODE>
     * @param props a sequenze of <CODE>PropertyVlaue</CODE>
     * @param pName the name of the property the value should be returned
     * @return the value of the property
     */
    protected Object getPropertyValueValue(PropertyValue[] props, String pName)
    {
        int i = 0;
        while (i < props.length && !props[i].Name.equals(pName))
        {
            i++;
        }
        return i < props.length ? props[i].Value : null;
    }

    /**
     * set a value of the specified (<CODE>pName</CODE>) property inside a sequenze of <CODE>PropertyValue</CODE>
     * @param props sequenze of <CODE>PropertyValue</CODE>
     * @param pName name of the property which should be changed
     * @param pValue the value the property should be assigned
     */
    protected void setPropertyValueValue(PropertyValue[] props, String pName, Object pValue)
    {
        int i = 0;
        while (i < props.length && !props[i].Name.equals(pName))
        {
            i++;
        }
        props[i].Value = pValue;
    }

    private XMultiServiceFactory getMSF()
    {
        return UnoRuntime.queryInterface(XMultiServiceFactory.class, connection.getComponentContext().getServiceManager());
    }

    // setup and close connections
    @BeforeClass
    public static void setUpConnection() throws Exception
    {
        System.out.println("setUpConnection()");
        connection.setUp();
    }

    @AfterClass
    public static void tearDownConnection()
            throws InterruptedException, com.sun.star.uno.Exception
    {
        System.out.println("tearDownConnection()");
        connection.tearDown();
    }
    private static final OfficeConnection connection = new OfficeConnection();
}
