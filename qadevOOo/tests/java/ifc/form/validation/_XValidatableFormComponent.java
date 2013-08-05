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
package ifc.form.validation;

import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyAttribute;
import com.sun.star.beans.XMultiPropertySet;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.form.validation.XFormComponentValidityListener;
import com.sun.star.form.validation.XValidatableFormComponent;
import com.sun.star.uno.UnoRuntime;

import lib.MultiMethodTest;

import util.ValueChanger;

import java.util.StringTokenizer;


public class _XValidatableFormComponent extends MultiMethodTest
{
    public XValidatableFormComponent oObj;
    protected XFormComponentValidityListener listener = null;
    public boolean listenerCalled = false;
    private String[] testPropsNames = null;
    private int testPropsAmount = 0;

    public void _addFormComponentValidityListener()
    {
        listener = new MyListener();

        boolean res = true;

        try
        {
            oObj.addFormComponentValidityListener(listener);
        }
        catch (com.sun.star.lang.NullPointerException e)
        {
            res = false;
            e.printStackTrace();
        }

        changeAllProperties();
        res &= listenerCalled;
        tRes.tested("addFormComponentValidityListener()", res);
    }

    public void _getCurrentValue()
    {
        oObj.getCurrentValue();
        tRes.tested("getCurrentValue()", true);
    }

    public void _isValid()
    {
        boolean res = oObj.isValid();
        tRes.tested("isValid()", res);
    }

    public void _removeFormComponentValidityListener()
    {
        requiredMethod("isValid()");

        try
        {
            oObj.removeFormComponentValidityListener(listener);
        }
        catch (com.sun.star.lang.NullPointerException e)
        {
            e.printStackTrace();
        }

        listenerCalled = false;
        changeAllProperties();
        tRes.tested("removeFormComponentValidityListener()", true);
    }

    protected void changeAllProperties()
    {
        XMultiPropertySet mProps =
            UnoRuntime.queryInterface(
            XMultiPropertySet.class, tEnv.getTestObject()
        );
        XPropertySetInfo propertySetInfo = mProps.getPropertySetInfo();
        Property[] properties = propertySetInfo.getProperties();
        getPropsToTest(properties);
        log.println("Changing all properties");

        Object[] gValues = mProps.getPropertyValues(testPropsNames);

        for (int i = 0; i < testPropsAmount; i++)
        {
            Object oldValue = gValues[i];

            if (
                testPropsNames[i].equals("Value")
                    || testPropsNames[i].equals("EffectiveValue")
            )
            {
                oldValue = new Integer(10);
            }
            else if (testPropsNames[i].equals("Time"))
            {
                oldValue = new com.sun.star.util.Time(
                    10, (short) 10, (short) 10, (short) 10, false);
            }

            Object newValue = ValueChanger.changePValue(oldValue);
            gValues[i] = newValue;

            //            System.out.println("#############################################");
            //            System.out.println("Name: "+testPropsNames[i]);
            //            System.out.println("OldValue: "+oldValue);
            //            System.out.println("NewValue: "+newValue);
            //            System.out.println("#############################################");
        }

        try
        {
            mProps.setPropertyValues(testPropsNames, gValues);
        }
        catch (com.sun.star.beans.PropertyVetoException e)
        {
            log.println("Exception occurred while setting properties");
            e.printStackTrace(log);
        }
        catch (com.sun.star.lang.IllegalArgumentException e)
        {
            log.println("Exception occurred while setting properties");
            e.printStackTrace(log);
        }
        catch (com.sun.star.lang.WrappedTargetException e)
        {
            log.println("Exception occurred while setting properties");
            e.printStackTrace(log);
        }
         // end of try-catch
    }

    //Get the properties being tested
    private void getPropsToTest(Property[] properties)
    {
        String bound = "";

        for (int i = 0; i < properties.length; i++)
        {
            Property property = properties[i];
            String name = property.Name;
            boolean isWritable =
                ((property.Attributes & PropertyAttribute.READONLY) == 0);
            boolean isNotNull =
                ((property.Attributes & PropertyAttribute.MAYBEVOID) == 0);
            //these have values that are interfaces we can't change
            if (
                name.equals("TextUserDefinedAttributes")
                    || name.equals("ReferenceDevice")
                    || name.equals("ParaUserDefinedAttributes")
            )
            {
                isWritable = false;
            }

            if (
                name.equals("Value") || name.equals("Time")
                    || name.equals("Date")
            )
            {
                bound = (name + ";");
            }

            if (
                isWritable && isNotNull && (name.indexOf("Format") < 0)
                    && !name.equals("Enabled")
            )
            {
                bound += (name + ";");
            }
        }
         // endfor

        //get a array of bound properties
        if (bound.equals(""))
        {
            bound = "none";
        }

        if (tEnv.getTestCase().getObjectName().indexOf("Formatted") > 0)
        {
            bound = "EffectiveValue;";
        }

        StringTokenizer ST = new StringTokenizer(bound, ";");
        int nr = ST.countTokens();
        testPropsNames = new String[nr];

        for (int i = 0; i < nr; i++)
            testPropsNames[i] = ST.nextToken();

        testPropsAmount = nr;

        return;
    }

    protected class MyListener implements XFormComponentValidityListener
    {
        public void componentValidityChanged(
            com.sun.star.lang.EventObject eventObject
        )
        {
            System.out.println("componentValidityChanged called");
            listenerCalled = true;
        }

        public void disposing(com.sun.star.lang.EventObject eventObject)
        {
            System.out.println("Listener Disposed");
        }
    }
}
