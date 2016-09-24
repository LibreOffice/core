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

import com.sun.star.container.XNameAccess;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

import util.utils;

import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;


public class TypeDetection6FileFormat
{

    XMultiServiceFactory xMSF;

    /** Create the environment for following tests.
     * Use either a component loader from desktop or
     * from frame
     * @throws Exception Exception
     */
    @Before public void before() throws Exception
    {
        xMSF = getMSF();
        assertNotNull("Could not get XMultiServiceFactory", xMSF);
    }

    /**
     * call the function <CODE>checkFileFormatSupport</CODE> to test <CODE>com.sun.star.document.FilterFactory</CODE>
     * @see com.sun.star.document.FilterFactory
     */
    @Test public void checkFilterFactory() throws Exception
    {
        checkFileFormatSupport("com.sun.star.document.FilterFactory");
    }

    /**
     * call the function <CODE>checkFileFormatSupport</CODE> to test <CODE>com.sun.star.document.TypeDetection</CODE>
     * @see com.sun.star.document.TypeDetection
     */
    @Test public void checkTypeDetection() throws Exception
    {
        checkFileFormatSupport("com.sun.star.document.TypeDetection");
    }

    /**
     * test the given service <CODE>serviceName</CODE>.
     * The serve was created and the filter 'TypeDetection6FileFormat' was searched
     * @param serviceName the name of the service to test
     */
    private void checkFileFormatSupport(String serviceName) throws Exception
    {
        System.out.println("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
        System.out.println("testing service '" + serviceName + "'");

        XInterface oObj = null;
        oObj = getTestObject(serviceName);
        System.out.println("ImplName: " + utils.getImplName(oObj));
        XNameAccess xNA = UnoRuntime.queryInterface(XNameAccess.class, oObj);
        String msg = "Could not find filter 'TypeDetection6FileFormat'!";
        msg += "\nMaybe 'TypeDetection6FileFormat.xcu' is not registered.";
        assertTrue(msg, xNA.hasByName("TypeDetection6FileFormat"));
    }

    /**
     * Creates an instance for the given <CODE>serviceName</CODE>
     * @param serviceName the name of the service which should be created
     * @throws Exception was thrown if creation fails
     * @return <CODE>XInterface</CODE> of service
     */
    public XInterface getTestObject(String serviceName) throws Exception
    {

        Object oInterface = xMSF.createInstance(serviceName);

        if (oInterface == null)
        {
            fail("Service wasn't created");
            throw new Exception("could not create service '" + serviceName + "'");
        }
        return (XInterface) oInterface;
    }

    private XMultiServiceFactory getMSF()
    {
        return UnoRuntime.queryInterface(XMultiServiceFactory.class, connection.getComponentContext().getServiceManager());
    }

    // setup and close connections
    @BeforeClass public static void setUpConnection() throws Exception {
        System.out.println("setUpConnection()");
        connection.setUp();
    }

    @AfterClass public static void tearDownConnection()
        throws InterruptedException, com.sun.star.uno.Exception
    {
        System.out.println("tearDownConnection()");
        connection.tearDown();
    }

    private static final OfficeConnection connection = new OfficeConnection();

}
