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
package complex.forms;

import static org.junit.Assert.assertTrue;

import java.util.ArrayList;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;

import util.FormTools;
import util.SOfficeFactory;
import util.ValueChanger;

import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyAttribute;
import com.sun.star.beans.PropertyChangeEvent;
import com.sun.star.beans.XMultiPropertySet;
import com.sun.star.beans.XPropertiesChangeListener;
import com.sun.star.drawing.XControlShape;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XCloseable;

/**
 */
public class CheckOGroupBoxModel
{

    private XMultiPropertySet m_xPropSet;
    private XComponent m_xDrawDoc;

    @Before public void before() throws Exception
    {
        SOfficeFactory SOF = SOfficeFactory.getFactory(getMSF());

        System.out.println("creating a draw document");
        m_xDrawDoc = SOF.createDrawDoc(null);

        String objName = "GroupBox";
        XControlShape shape = FormTools.insertControlShape(m_xDrawDoc, 5000, 7000, 2000, 2000, objName);
        m_xPropSet = UnoRuntime.queryInterface(XMultiPropertySet.class, shape.getControl());
    }

    @After public void after() throws Exception
    {
        XCloseable xClose = UnoRuntime.queryInterface(XCloseable.class, m_xDrawDoc);
        if (xClose != null)
        {
            xClose.close(true);
        }
    }

    @Test public void setPropertyValues() throws Exception
    {
        String[] boundPropsToTest = getBoundPropsToTest();

        MyChangeListener ml = new MyChangeListener();
        m_xPropSet.addPropertiesChangeListener(boundPropsToTest, ml);

        Object[] gValues = m_xPropSet.getPropertyValues(boundPropsToTest);
        Object[] newValue = new Object[gValues.length];
        System.out.println("Trying to change all properties.");
        for (int i = 0; i < boundPropsToTest.length; i++)
        {
            newValue[i] = ValueChanger.changePValue(gValues[i]);
        }
        m_xPropSet.setPropertyValues(boundPropsToTest, newValue);

        assertTrue("Listener was not called.", ml.wasListenerCalled());
        m_xPropSet.removePropertiesChangeListener(ml);
    }

    private String[] getBoundPropsToTest()
    {
        Property[] properties = m_xPropSet.getPropertySetInfo().getProperties();
        ArrayList<String> tNames = new ArrayList<String>();

        for (Property property : properties)
        {
            boolean isWritable = ((property.Attributes
                    & PropertyAttribute.READONLY) == 0);
            boolean isNotNull = ((property.Attributes
                    & PropertyAttribute.MAYBEVOID) == 0);
            boolean isBound = ((property.Attributes
                    & PropertyAttribute.BOUND) != 0);

            if (isWritable && isNotNull && isBound)
            {
                tNames.add(property.Name);
            }
        } // endfor

        //get a array of bound properties
        String[] testPropsNames = tNames.toArray(new String[tNames.size()]);
        return testPropsNames;
    }

    /**
     * Listener implementation which sets a flag when
     * listener was called.
     */
    private static class MyChangeListener implements XPropertiesChangeListener
    {

        private boolean propertiesChanged = false;

        public void propertiesChange(PropertyChangeEvent[] e)
        {
            propertiesChanged = true;
        }

        public void disposing(EventObject obj)
        {
        }

        private boolean wasListenerCalled()
        {
            return propertiesChanged;
        }


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
