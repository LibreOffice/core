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
package complex.path_settings;

import com.sun.star.beans.Property;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XFastPropertySet;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertiesChangeListener;
import com.sun.star.beans.XPropertyChangeListener;
import com.sun.star.beans.XVetoableChangeListener;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.AnyConverter;

// ---------- junit imports -----------------
import java.util.ArrayList;
import java.util.StringTokenizer;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;
// ------------------------------------------

public class PathSettingsTest
{

    private static XMultiServiceFactory xMSF;
    // the test object: an instance of the tested service
    private static Object aPathSettings = null;
    // the properties of the service
    private static Property[] xPropertyInfoOfPathSettings = null;
    private static String[] aPathSettingNames = null;
    private static String[] availablePropNames = new String[]
    {
        "Addin",
        "AutoCorrect",
        "AutoText",
        "Backup",
        "Basic",
        "Bitmap",
        "Config",
        "Dictionary",
        "Favorite",
        "Filter",
        "Fingerprint",
        "Gallery",
        "Graphic",
        "Help",
        "Linguistic",
        "Module",
        "Palette",
        "Plugin",
        "Storage",
        "Temp",
        "Template",
        "UIConfig",
        "UserConfig",
        "Work",
    };
    // every path name from availablePropNames exist in this characteristics
    // name
    // name_internal
    // name_user
    // name_writable
    private static String[] availablePropNameExtensions = new String[]
    {
        "",
        "_internal",
        "_user",
        "_writable"
    };
    private static String[] aPathSettingValues = null;
    ArrayList<Property> aListOfWorkingProperty;

    /**
     * A function to tell the framework, which test functions are available.
     * Right now, it's only 'checkComplexTemplateState'.
     * @return All test methods.
     */
//    public String[] getTestMethodNames() {
//        return new String[]{"checkXFastPropertySet",
//                            "checkXMultiPropertySet",
//                            "checkXPropertySet"
//                        };
//    }
    /**
     * Initialize before the tests start: this has to be done only once.
     * This methods sets the 'aPathSettings' and 'xPropertyInfoOfPathSettings' variables.
     */
    @Before
    public void before()
    {
        try
        {
            xMSF = getMSF();
            aPathSettings = xMSF.createInstance("com.sun.star.comp.framework.PathSettings");
            assertNotNull("Can't instantiate com.sun.star.util.PathSettings.", aPathSettings);
//            System.out.println("Implementation: " + util.utils.getImplName(aPathSettings));
//            System.out.println("Service:        ");
            util.dbg.getSuppServices(aPathSettings);
            final XPropertySet xPropSet_of_PathSettings = UnoRuntime.queryInterface(XPropertySet.class, aPathSettings);

            xPropertyInfoOfPathSettings = xPropSet_of_PathSettings.getPropertySetInfo().getProperties();
            aPathSettingNames = new String[xPropertyInfoOfPathSettings.length];
            aPathSettingValues = new String[xPropertyInfoOfPathSettings.length];

            aListOfWorkingProperty = new ArrayList<Property>();

            // get intitial values and create new ones
            for (int i = 0; i < xPropertyInfoOfPathSettings.length; i++)
            {
                final String sName = xPropertyInfoOfPathSettings[i].Name;
                // System.out.println(sName);
                aPathSettingNames[i] = sName;
                Object o = xPropSet_of_PathSettings.getPropertyValue(sName);

                String sValue = convertToString(o);
                aPathSettingValues[i] = sValue;
                aListOfWorkingProperty.add(xPropertyInfoOfPathSettings[i]);
            }
        }
        catch (com.sun.star.uno.Exception e)
        {
            System.out.println(e.getClass().getName());
            System.out.println("Message: " + e.getMessage());
            // fail("Could not create an instance of the test object.");
        }
        catch (Exception e)
        {
            fail("What exception?");
        }
    }

    private String convertToString(Object o)
    {
        String sValue = "";
        try
        {
            if (AnyConverter.isString(o))
            {
                sValue = AnyConverter.toString(o);
            }
            else if (AnyConverter.isArray(o))
            {
                Object oValueList = AnyConverter.toArray(o);
                String[] aValueList = (String[]) oValueList;
                String sValues = "";
                for (int j = 0; j < aValueList.length; j++)
                {
                    if (sValues.length() > 0)
                    {
                        sValues += ";";
                    }
                    sValues += aValueList[j];
                }
                sValue = sValues;
            }
            else
            {
                System.out.println("Can't convert Object to String");
            }
        }
        catch (com.sun.star.uno.Exception e)
        {
            /* ignore */
        }
        return sValue;
    }

    /**
     * Simple existance test, if this fails, the Lists must update
     */
    @Test
    public void checkInternalListConsistence()
    {
        // check if all Properties are in the internal test list
        for (int i = 0; i < xPropertyInfoOfPathSettings.length; i++)
        {
            final String sName = xPropertyInfoOfPathSettings[i].Name;
            boolean bOccur = checkIfNameExistsInList(sName, availablePropNames, availablePropNameExtensions);
            assertTrue("TEST IS WRONG, Name:='" + sName + "' doesn't exist in internal Test list.", bOccur);
        }

        // check if all properties in the internal list also exist in real life
        for (int i = 0; i < availablePropNames.length; i++)
        {
            final String aListName = availablePropNames[i];
            for (int j = 0; j < availablePropNameExtensions.length; j++)
            {
                final String aSubListName = availablePropNameExtensions[j];
                final String aName = aListName + aSubListName;
                boolean bOccur = checkIfNameExistsInList(aName, aPathSettingNames, new String[]
                        {
                            ""
                        } /* list must not empty! */);
                assertTrue("TEST IS WRONG, Name:='" + aName + "' from the internal test list do not occur in real life path settings.", bOccur);
            }
        }
    }

    /**
     * Simple O(n^n) check if a given String (_sNameMustOccur) exist in the given list(+SubList) values.
     * @param _sNameMustOccur
     * @param _aList
     * @param _aSubList
     * @return true, if name occur
     */
    private boolean checkIfNameExistsInList(String _sNameMustOccur, String[] _aList, String[] _aSubList)
    {
        for (int i = 0; i < _aList.length; i++)
        {
            final String aListName = _aList[i];
            for (int j = 0; j < _aSubList.length; j++)
            {
                final String aSubListName = _aSubList[j];
                final String aName = aListName + aSubListName;
                if (aName.equals(_sNameMustOccur))
                {
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * Shows the path settings
     * @throws UnknownPropertyException
     * @throws WrappedTargetException
     */
    @Test
    public void showPathSettings() throws UnknownPropertyException, WrappedTargetException
    {
        System.out.println("\n---- All properties ----");
        final XPropertySet xPropSet_of_PathSettings = UnoRuntime.queryInterface(XPropertySet.class, aPathSettings);

//        for (int i = 0; i < xPropertyInfoOfPathSettings.length; i++)
        for (int i = 0; i < aListOfWorkingProperty.size(); i++)
        {
            final String sName = aListOfWorkingProperty.get(i).Name;
            // aPathSettingWorkingNames[i] = sName;
//            System.out.print("PathSettings: Name:=");
            System.out.print(sName);
            Object o = xPropSet_of_PathSettings.getPropertyValue(sName);

            // System.out.println("#### Object: '" + o.getClass().getName() + "'  -  '" + o.toString() + "'");
            try
            {
                final String sValue = AnyConverter.toString(o);
                // aPathSettingValues[i] = sValue;
                // System.out.println("#### String " + sValue);
                // System.out.println("Property Name: " + sName);
                // System.out.println("Property Value: " + sValue);
//                System.out.print(" ==> ");
//                System.out.print(sValue);
            }
            catch (com.sun.star.uno.Exception e)
            {
//                System.out.print(" FAILED ");
            }
            System.out.println();
        }
        System.out.println("---- Finish showing properties ----\n");
    }

    private boolean checkPaths(Object _o, Object _o2)
    {
        String sLeftPath = "";
        String sRightPath = "";
        if (AnyConverter.isArray(_o))
        {
            try
            {
                Object oValues = AnyConverter.toArray(_o);
                sLeftPath = convertToString(oValues);
            }
            catch (com.sun.star.lang.IllegalArgumentException e)
            {
            }
        }
        else if (AnyConverter.isString(_o))
        {
            try
            {
                sLeftPath = AnyConverter.toString(_o);
            }
            catch (com.sun.star.lang.IllegalArgumentException e)
            {
            }
        }

        if (AnyConverter.isArray(_o2))
        {
            try
            {
                Object oValues = AnyConverter.toArray(_o2);
                sRightPath = convertToString(oValues);
            }
            catch (com.sun.star.lang.IllegalArgumentException e)
            {
            }
        }
        else if (AnyConverter.isString(_o2))
        {
            try
            {
                sRightPath = AnyConverter.toString(_o2);
            }
            catch (com.sun.star.lang.IllegalArgumentException e)
            {
            }
        }
        return checkPaths(sLeftPath, sRightPath);
    }

    /**
     * Check 2 given paths if the _aOtherPath exists in _aPath, _aPath could be a list separated by ';'
     * @param _aPath
     * @param _aOtherPath
     * @return true, if _aOtherPath found
     */
    private boolean checkPaths(String _aPath, String _aOtherPath)
    {
        if (_aOtherPath.contains(";"))
        {
            StringTokenizer aToken = new StringTokenizer(_aOtherPath, ";");
            int nCount = 0;
            int nFound = 0;
            while (aToken.hasMoreElements())
            {
                String sPath = (String)aToken.nextElement();
                nCount ++;
                if (checkPaths(_aPath, sPath))
                {
                    nFound++;
                }
            }
            if (nFound == nCount)
            {
                return true;
            }
        }
        else if(_aPath.contains(";"))
        {
            StringTokenizer aToken = new StringTokenizer(_aPath, ";");
            while (aToken.hasMoreElements())
            {
                String sToken = (String)aToken.nextElement();
                if (sToken.equals(_aOtherPath))
                {
                    return true;
                }
            }
            return false;
        }
        else if (_aPath.equals(_aOtherPath))
        {
            return true;
        }
        return false;
    }

    /**
     * This tests the XFastPropertySet interface implementation.
     */
    @Test
    public void checkXFastPropertySet()
    {
        System.out.println("---- Testing the XFastPropertySet interface ----");


        // do for all properties
        // xPropertyInfoOfPathSettings.length
        for (int i = 0; i < aListOfWorkingProperty.size(); i++)
        {
            final Property property = aListOfWorkingProperty.get(i); // xPropertyInfoOfPathSettings[i];
            String name = property.Name;
            // get property name and initial value
            System.out.println("Test property with name: " + name);
            boolean bResult;
            if (name.endsWith("_writable"))
            {
                bResult = checkStringProperty(property);
            }
            else if (name.endsWith("_user"))
            {
                bResult = checkStringListProperty(property);
            }
            else if (name.endsWith("_internal"))
            {
                bResult = checkStringListProperty(property);
            }
            else
            {
                // old path settings
                bResult = checkStringProperty(property);
            }
            System.out.print(" Test of property " + name + " finished");
            if (bResult)
            {
                System.out.println(" [ok]");
            }
            else
            {
                System.out.println(" [FAILED]");
            }
            System.out.println();
        }
        System.out.println("---- Test of XFastPropertySet finished ----\n");
    }

    private boolean checkStringListProperty(Property property)
    {
        // creating instances
        boolean bResult = true;
        XFastPropertySet xFPS = UnoRuntime.queryInterface(XFastPropertySet.class, aPathSettings);

        String name = property.Name;
        int handle = property.Handle;

        Object oValue;
        try
        {
            oValue = xFPS.getFastPropertyValue(handle);
        }
        catch (UnknownPropertyException ex)
        {
            return false;
        }
        catch (WrappedTargetException ex)
        {
            return false;
        }

        if (!AnyConverter.isArray(oValue))
        {
            System.out.println(" Internal error, type wrong. PathSetting property with name:" + name + " should be an array.");
            return false;
        }

        String val;
        try
        {
            Object oValues = AnyConverter.toArray(oValue);


            final String[] aValues = (String[])oValues;

            // aNewValues contains a deep copy of aValues
            String[] aNewValues = new String[aValues.length];
            System.arraycopy(aValues, 0, aNewValues, 0, aValues.length);
            if (aValues.length > 0)
            {
                val = aValues[0];
            }
            else
            {
                val = null;
                aNewValues = new String[1]; // create a String list
            }
            System.out.println(" Property has initial value: '" + val + "'");

            // set to a new correct value
            String newVal = changeToCorrectValue(val);
            assertFalse("newVal must not equal val.", newVal.equals(val));

            System.out.println(" Try to change to a correct value '" + newVal + "'");
            aNewValues[0] = newVal;

            try
            {
                try
                {
                    xFPS.setFastPropertyValue(handle, aNewValues);
                }
                catch (com.sun.star.lang.WrappedTargetException e)
                {
                    System.out.println(" FAIL: setFastPropertyValue(handle:=" + handle + ", name:='" + name + "')" + e.getMessage());
                    bResult = false;
                }

                // Property_internal can't change we will not arrive bejond this line

                // check the change
                Object oObj = xFPS.getFastPropertyValue(handle);
                if (!checkPaths(oObj, aNewValues))
                {
                    System.out.println(" FAIL: Did not change value on property " + name + ".");
                    bResult = false;
                }

                // set back to initial setting
                System.out.println(" Try to check");
                try
                {
                    xFPS.setFastPropertyValue(handle, oValue);
                }
                catch (com.sun.star.beans.PropertyVetoException e)
                {
                    // should not occur
                    System.out.println(" FAIL: PropertyVetoException caught: " + e.getMessage());
                    bResult = false;
                }
            }
            catch (com.sun.star.beans.PropertyVetoException e)
            {
                if (!name.endsWith("_internal"))
                {
                    // should not occur
                   System.out.println(" FAIL: PropertyVetoException caught: " + e.getMessage());
                   bResult = false;
                }
                else
                {
                   System.out.println(" OK: PropertyVetoException caught: " + e.getMessage() + " it seems not allowed to change internal values.");
                }
            }

            // check if changed
            Object checkVal3 = xFPS.getFastPropertyValue(handle);
            if (!checkPaths(checkVal3, oValues))
            {
                System.out.println(" FAIL: Can't change value back to original on property " + name);
                bResult = false;
            }
        }
        catch (com.sun.star.uno.Exception e)
        {
            System.out.println(" FAIL: getFastPropertyValue(handle:=" + handle + ", name:='" + name + "')" + e.getMessage());
            bResult = false;
        }
        return bResult;
    }

    private boolean checkStringProperty(Property property)
    {
        boolean bResult = true;
        XFastPropertySet xFPS = UnoRuntime.queryInterface(XFastPropertySet.class, aPathSettings);
        String name = property.Name;
        int handle = property.Handle;
        Object oValue;
        try
        {
            oValue = xFPS.getFastPropertyValue(handle);
        }
        catch (UnknownPropertyException ex)
        {
            return false;
        }
        catch (WrappedTargetException ex)
        {
            return false;
        }


        try
        {
            String val = "";
            val = AnyConverter.toString(oValue);
            System.out.println(" Property has initial value: '" + val + "'");

            // set to a new correct value
            String newVal = changeToCorrectValue(val);
            System.out.println(" Try to change to a correct value '" + newVal + "'");
            xFPS.setFastPropertyValue(handle, newVal);

            // check the change
            String checkVal = (String) xFPS.getFastPropertyValue(handle);
            if (!checkPaths(checkVal, newVal))
            {
                System.out.println("  FAIL: Did not change value on property " + name + ".");
                bResult = false;
            }
            newVal = changeToIncorrectValue(val);
            System.out.println(" Try to change to incorrect value '" + newVal + "'");
            try
            {
                xFPS.setFastPropertyValue(handle, newVal);
            }
            catch (com.sun.star.lang.IllegalArgumentException e)
            {
                System.out.println("  Correctly thrown Exception caught.");
            }

            // check if changed
            String checkVal2 = (String) xFPS.getFastPropertyValue(handle);
            if (!checkPaths(checkVal2, checkVal))
            {
                System.out.println("  FAIL: Value did change on property " + name + " though it should not have.");
                bResult = false;
            }
            else
            {
                System.out.println("  OK: Incorrect value was not set.");
            }
            // set back to initial setting
            System.out.println(" Set back to initial value.");
            try
            {
                xFPS.setFastPropertyValue(handle, val);
            }
            catch (com.sun.star.lang.IllegalArgumentException e)
            {
                System.out.println("  IllegalArgumentException caught: " + e.getMessage());
                bResult = false;
            }
            // check if changed
            String checkVal3 = (String) xFPS.getFastPropertyValue(handle);
            if (!checkVal3.equals(val))
            {
                if (!checkPaths(checkVal3, val))
                {
                    System.out.println("  FAIL: Can't change value back to original on property " + name);
                    System.out.println("  Value is: " + checkVal3);

                    bResult = false;
                }
                else
                {
                    System.out.println("  OK: the pathsettings contains the original path.");
                    System.out.println("         Value is: " + checkVal3);
                    System.out.println("  Value should be: " + val);
                }
            }
            else
            {
                System.out.println("  OK: Change value back to original on property " + name);
            }
        }
        catch (com.sun.star.uno.Exception e)
        {
            System.out.println(" FAIL: getFastPropertyValue(handle:=" + handle + ", name:='" + name + "')" + e.getMessage());
            bResult = false;
        }
        return bResult;
    }

    // ____________________
    /**
     * This tests the XMultiPropertySet interface implementation.
     */

    // The test checkXMultiPropertySet() has been marked as outdated!

//    @Test
//    public void checkXMultiPropertySet()
//    {
//        System.out.println("---- Testing the XMultiPropertySet interface ----");
//        XMultiPropertySet xMPS = UnoRuntime.queryInterface(XMultiPropertySet.class, aPathSettings);
//
//        // xPropertyInfoOfPathSettings.length
//        String[] propertiesToTest = new String[1];
//        propertiesToTest[0] = availablePropNames[0];
//
//        String[] correctVals = new String[propertiesToTest.length];
//        String[] incorrectVals = new String[propertiesToTest.length];
//
//        String[] aPathSettingWorkingNames = null;
//        aPathSettingWorkingNames = new String[propertiesToTest.length];
//
//        // get intitial values and create new ones
//        for (int i = 0; i < propertiesToTest.length; i++)
//        {
//            // Property aProp = aListOfWorkingProperty.get(i);
//            final String sName = propertiesToTest[i];
//            final String sValue = getPropertyValueAsString(sName);
//            aPathSettingWorkingNames[i] = sName;
//            correctVals[i] = changeToCorrectValue(sValue);
//            incorrectVals[i] = changeToIncorrectValue(sValue);
//        }
//
//        try
//        {
//            // add a change listener
//            MyChangeListener mListener = new MyChangeListener();
//            xMPS.addPropertiesChangeListener(aPathSettingWorkingNames, mListener);
//
//            // first change xPropertyInfoOfPathSettings to correct values
//            System.out.println("Change to correct values.");
//            xMPS.setPropertyValues(aPathSettingWorkingNames, correctVals);
//            assertTrue("Could not change to correct values with XMultiPropertySet.",
//                    verifyPropertySet(xMPS, aPathSettingWorkingNames, correctVals) > 0);
//
//            // second, change to incorrect values: expect an exception
//            System.out.println("Try to change to incorrect values.");
//            try
//            {
//                xMPS.setPropertyValues(aPathSettingWorkingNames, incorrectVals);
//            }
//            catch (com.sun.star.lang.IllegalArgumentException r)
//            {
//                System.out.println("Correctly thrown Exception caught.");
//            }
//            assertTrue("Did change to incorrect values with XMultiPropertySet,"
//                    + " but should not have.",
//                    verifyPropertySet(xMPS, aPathSettingWorkingNames, correctVals) > 0);
//
//            // third, change back to initial values
//            System.out.println("Change back to initial values.");
//            xMPS.setPropertyValues(aPathSettingWorkingNames, aPathSettingValues);
//            assertTrue("Could not change back to initial values with"
//                    + " XMultiPropertySet.",
//                    verifyPropertySet(xMPS, aPathSettingWorkingNames, aPathSettingValues) > 0);
//
//            // fire the event for the listener
//            System.out.println("Fire event.");
//            xMPS.firePropertiesChangeEvent(aPathSettingWorkingNames, mListener);
//            assertTrue("Event was not fired on XMultiPropertySet.",
//                    mListener.changePropertiesEventFired());
//        }
//        catch (com.sun.star.uno.Exception e)
//        {
////            e.printStackTrace();
//            System.out.println(e.getClass().getName());
//            System.out.println("Message: " + e.getMessage());
//            fail("Unexpected exception on XMultiPropertySet.");
//        }
//
//        // test finished
//        System.out.println("---- Test of XMultiPropertySet finished ----\n");
//    }

    /**
     * Verify if the values of xPropSet_of_PathSettings are the same as vals.
     * @param xPropSet_of_PathSettings A XMultiPropertySet.
     * @param aPathSettingWorkingNames An array with property names.
     * @param vals An array with values of the properties
     * @return -1 if none are equal, 1 if all are equal, 0 if some were equal
     * and some not.
     * @throws com.sun.star.lang.IllegalArgumentException
     */
//    private int verifyPropertySet(XMultiPropertySet xProp,
//            String[] propNames, String[] vals)
//    {
//        int ret = 0;
//        if (vals.length != propNames.length)
//        {
//            System.out.println("Length of array parameters must be equal.");
//            return ret;
//        }
//        for (int i = 0; i < vals.length; i++)
//        {
//            Object[] objs = xProp.getPropertyValues(new String[]
//                    {
//                        propNames[i]
//                    });
//            String retVal = (String) objs[0];
//            boolean nCheck = retVal.equals(vals[i]);
//            if (!nCheck)
//            {
//                System.out.println("Property '" + propNames[i]
//                        + "' was supposed to have value:");
//                System.out.println(vals[i]);
//                System.out.println("but has value:");
//                System.out.println(retVal);
//            }
//            // initialize
//            if (i == 0)
//            {
//                ret = nCheck ? 1 : -1;
//                continue;
//            }
//            // return 0 if equal state changes compared to initial value
//            if ((nCheck && ret < 0) || (!nCheck && ret > 0))
//            {
//                ret = 0;
//            }
//        }
//        return ret;
//    }

    // ____________________
    /**
     * This tests the XPropertySet interface implementation.
     */

// The test checkXPropertySet() has been marked as outdated!


//    @Test
//    public void checkXPropertySet()
//    {
//        System.out.println("---- Testing the XPropertySet interface ----");
//
//        XPropertySet xPS = UnoRuntime.queryInterface(XPropertySet.class, aPathSettings);
//
//        MyChangeListener mListener1 = new MyChangeListener();
//        MyChangeListener mListener2 = new MyChangeListener();
//
//        for (int i = 0; i < xPropertyInfoOfPathSettings.length; i++)
//        {
//            // adding listeners
//            String name = aPathSettingNames[i];
//            System.out.println("Testing property '" + name + "'");
//            try
//            {
//                System.out.println("Add 2 Listeners.");
//                xPS.addPropertyChangeListener(name, mListener1);
//                xPS.addVetoableChangeListener(name, mListener1);
//                xPS.addPropertyChangeListener(name, mListener2);
//                xPS.addVetoableChangeListener(name, mListener2);
//
//                // change the property
//                System.out.println("Change value.");
//                String changeVal = changeToCorrectValue(aPathSettingValues[i]);
//                xPS.setPropertyValue(name, changeVal);
//                String newVal = (String) xPS.getPropertyValue(name);
//
//                assertTrue("Value did not change on property " + name + ".",
//                        newVal.equals(changeVal));
//
//                assertTrue("Listener 1 was not called.", checkListener(mListener1));
//                assertTrue("Listener 2 was not called.", checkListener(mListener2));
//
//                mListener1.resetListener();
//                mListener2.resetListener();
//
//                System.out.println("Remove Listener 1.");
//
//                xPS.removePropertyChangeListener(name, mListener1);
//                xPS.removeVetoableChangeListener(name, mListener1);
//
//                // change the property
//                System.out.println("Change value back.");
//                xPS.setPropertyValue(name, aPathSettingValues[i]);
//                newVal = (String) xPS.getPropertyValue(name);
//                assertTrue("Value did not change on property " + name,
//                        newVal.equals(aPathSettingValues[i]));
//
//                assertTrue("Listener was called, although it was removed on"
//                        + " property " + name + ".", !checkListener(mListener1));
//                assertTrue("Listener 2 was not called on property " + name + ".",
//                        checkListener(mListener2));
//            }
//            catch (com.sun.star.uno.Exception e)
//            {
//                System.out.println(e.getClass().getName());
//                System.out.println("Message: " + e.getMessage());
//                fail("Unexpcted exception on property " + name);
//            }
//            System.out.println("Finish testing property '" + aPathSettingNames[i] + "'\n");
//        }
//        System.out.println("---- Test of XPropertySet finished ----\n");
//
//    }

//    private boolean checkListener(MyChangeListener ml)
//    {
//        return ml.changePropertyEventFired()
//                || ml.changePropertiesEventFired()
//                || ml.vetoableChangeEventFired();
//    }

    // ____________________
    /**
     * Change the given String to a correct path URL.
     * @return The changed path URL.
     */
    private String changeToCorrectValue(String path)
    {
        // the simplest possiblity
        if (path == null || path.equals(""))
        {
            String sTempDir = System.getProperty("java.io.tmpdir");
            sTempDir = util.utils.getFullURL(sTempDir);
            return sTempDir; // "file:///tmp";
        }
        return graphical.FileHelper.appendPath(path, "tmp");
    }

    /**
     * Change the given String to an incorrect path URL.
     * @return The changed path URL.
     */
    private String changeToIncorrectValue(String path)
    {
        // return an illegal path
        return "fileblablabla";
    }

    /**
     * Listener implementation which sets a flag when
     * listener was called.
     */
    public class MyChangeListener implements XPropertiesChangeListener,
            XPropertyChangeListener,
            XVetoableChangeListener
    {

        private boolean propChanged = false;
        private boolean propertiesChanged = false;
        private boolean vetoableChanged = false;

        public void propertiesChange(
                com.sun.star.beans.PropertyChangeEvent[] e)
        {
            propertiesChanged = true;
        }

        public void vetoableChange(com.sun.star.beans.PropertyChangeEvent pE)
                throws com.sun.star.beans.PropertyVetoException
        {
            vetoableChanged = true;
        }

        public void propertyChange(com.sun.star.beans.PropertyChangeEvent pE)
        {
            propChanged = true;
        }

        public void disposing(com.sun.star.lang.EventObject eventObject)
        {
        }

        public void resetListener()
        {
            propChanged = false;
            propertiesChanged = false;
            vetoableChanged = false;
        }

        public boolean changePropertyEventFired()
        {
            return propChanged;
        }

        public boolean changePropertiesEventFired()
        {
            return propertiesChanged;
        }

        public boolean vetoableChangeEventFired()
        {
            return vetoableChanged;
        }
    }

    private XMultiServiceFactory getMSF()
    {
        final XMultiServiceFactory xMSF1 = UnoRuntime.queryInterface(XMultiServiceFactory.class, connection.getComponentContext().getServiceManager());
        return xMSF1;
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
