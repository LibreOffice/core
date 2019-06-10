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
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.AnyConverter;
import com.sun.star.util.thePathSettings;

// ---------- junit imports -----------------
import java.util.ArrayList;
import java.util.StringTokenizer;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;


public class PathSettingsTest
{

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
     * Initialize before the tests start: this has to be done only once.
     * This methods sets the 'aPathSettings' and 'xPropertyInfoOfPathSettings' variables.
     */
    @Before
    public void before() throws Exception
    {
        try
        {
            aPathSettings = thePathSettings.get(connection.getComponentContext());
            assertNotNull("Can't instantiate com.sun.star.util.thePathSettings.", aPathSettings);
            util.dbg.getSuppServices(aPathSettings);
            final XPropertySet xPropSet_of_PathSettings = UnoRuntime.queryInterface(XPropertySet.class, aPathSettings);

            xPropertyInfoOfPathSettings = xPropSet_of_PathSettings.getPropertySetInfo().getProperties();
            aPathSettingNames = new String[xPropertyInfoOfPathSettings.length];
            aPathSettingValues = new String[xPropertyInfoOfPathSettings.length];

            aListOfWorkingProperty = new ArrayList<Property>();

            // get initial values and create new ones
            for (int i = 0; i < xPropertyInfoOfPathSettings.length; i++)
            {
                final String sName = xPropertyInfoOfPathSettings[i].Name;
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
        catch (com.sun.star.lang.IllegalArgumentException e)
        {
            /* ignore */
        }
        return sValue;
    }

    /**
     * Simple existence test, if this fails, the Lists must update
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
     */
    @Test
    public void showPathSettings() throws UnknownPropertyException, WrappedTargetException
    {
        System.out.println("\n---- All properties ----");
        final XPropertySet xPropSet_of_PathSettings = UnoRuntime.queryInterface(XPropertySet.class, aPathSettings);

        for (int i = 0; i < aListOfWorkingProperty.size(); i++)
        {
            final String sName = aListOfWorkingProperty.get(i).Name;
            System.out.print(sName);
            Object o = xPropSet_of_PathSettings.getPropertyValue(sName);

            try
            {
                AnyConverter.toString(o);
            }
            catch (com.sun.star.lang.IllegalArgumentException e)
            {
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

                // Property_internal can't change we will not arrive beyond this line

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
            /*
             * Change the given String to an incorrect path URL.
             */
            newVal = "fileblablabla";
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
     * This tests the XPropertySet interface implementation.
     */

    /**
     * Change the given String to a correct path URL.
     * @return The changed path URL.
     */
    private String changeToCorrectValue(String path)
    {
        // the simplest possibility
        if (path == null || path.equals(""))
        {
            String sTempDir = System.getProperty("java.io.tmpdir");
            sTempDir = util.utils.getFullURL(sTempDir);
            return sTempDir; // "file:///tmp";
        }
        return graphical.FileHelper.appendPath(path, "tmp");
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
