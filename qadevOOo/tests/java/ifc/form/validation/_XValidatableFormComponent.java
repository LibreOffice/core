/*************************************************************************
 *
 *  $RCSfile: _XValidatableFormComponent.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2004-11-16 12:50:49 $
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
        Object cValue = oObj.getCurrentValue();
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

        boolean res = true;

        try
        {
            oObj.removeFormComponentValidityListener(listener);
        }
        catch (com.sun.star.lang.NullPointerException e)
        {
            res = false;
            e.printStackTrace();
        }

        listenerCalled = false;
        changeAllProperties();
        res &= !listenerCalled;
        tRes.tested("removeFormComponentValidityListener()", true);
    }

    protected void changeAllProperties()
    {
        XMultiPropertySet mProps =
            (XMultiPropertySet) UnoRuntime.queryInterface(
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
                    || testPropsNames[i].equals("Time")
                    || testPropsNames[i].equals("EffectiveValue")
            )
            {
                oldValue = new Integer(10);
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
            log.println("Exception occured while setting properties");
            e.printStackTrace(log);
        }
        catch (com.sun.star.lang.IllegalArgumentException e)
        {
            log.println("Exception occured while setting properties");
            e.printStackTrace(log);
        }
        catch (com.sun.star.lang.WrappedTargetException e)
        {
            log.println("Exception occured while setting properties");
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
            boolean isBound =
                ((property.Attributes & PropertyAttribute.BOUND) != 0);

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
