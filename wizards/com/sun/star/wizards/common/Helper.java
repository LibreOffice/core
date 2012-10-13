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
package com.sun.star.wizards.common;

import com.sun.star.uno.XComponentContext;
import com.sun.star.util.XMacroExpander;
import java.util.Calendar;

import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.Locale;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.RuntimeException;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.DateTime;
import com.sun.star.util.XNumberFormatsSupplier;
import com.sun.star.util.XNumberFormatter;

public class Helper
{

    /** Creates a new instance of Helper */
    public Helper()
    {
    }

    public static long convertUnoDatetoInteger(com.sun.star.util.Date DateValue)
    {
        java.util.Calendar oCal = java.util.Calendar.getInstance();
        oCal.set(DateValue.Year, DateValue.Month, DateValue.Day);
        java.util.Date dTime = oCal.getTime();
        long lTime = dTime.getTime();
        return lTime / (3600 * 24000);
    }

    public static void setUnoPropertyValue(Object oUnoObject, String PropertyName, Object PropertyValue)
    {
        try
        {
            XPropertySet xPSet = UnoRuntime.queryInterface(XPropertySet.class, oUnoObject);
            if (xPSet.getPropertySetInfo().hasPropertyByName(PropertyName))
            {
                xPSet.setPropertyValue(PropertyName, PropertyValue);
            }
            else
            {
                Property[] selementnames = xPSet.getPropertySetInfo().getProperties();
                throw new java.lang.IllegalArgumentException("No Such Property: '" + PropertyName + "'");
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }

    public static Object getUnoObjectbyName(Object oUnoObject, String ElementName)
    {
        try
        {
            com.sun.star.container.XNameAccess xName = UnoRuntime.queryInterface(com.sun.star.container.XNameAccess.class, oUnoObject);
            if (xName.hasByName(ElementName))
            {
                return xName.getByName(ElementName);
            }
            else
            {
                throw new RuntimeException();
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
            return null;
        }
    }

    public static Object getPropertyValue(PropertyValue[] CurPropertyValue, String PropertyName)
    {
        int MaxCount = CurPropertyValue.length;
        for (int i = 0; i < MaxCount; i++)
        {
            if (CurPropertyValue[i] != null)
            {
                if (CurPropertyValue[i].Name.equals(PropertyName))
                {
                    return CurPropertyValue[i].Value;
                }
            }
        }
        throw new RuntimeException();
    }

    public static Object getUnoPropertyValue(Object oUnoObject, String PropertyName, java.lang.Class<?> xClass)
    {
        try
        {
            if (oUnoObject != null)
            {
                XPropertySet xPSet = UnoRuntime.queryInterface(XPropertySet.class, oUnoObject);
                Object oObject = xPSet.getPropertyValue(PropertyName);
                if (AnyConverter.isVoid(oObject))
                {
                    return null;
                }
                else
                {
                    return com.sun.star.uno.AnyConverter.toObject(new com.sun.star.uno.Type(xClass), oObject);
                }
            }
            return null;
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
            return null;
        }
    }

    public static Object getPropertyValuefromAny(Object[] CurPropertyValue, String PropertyName)
    {
        if (CurPropertyValue != null)
        {
            int MaxCount = CurPropertyValue.length;
            for (int i = 0; i < MaxCount; i++)
            {
                if (CurPropertyValue[i] != null)
                {
                    PropertyValue aValue = (PropertyValue) CurPropertyValue[i];
                    if (aValue != null && aValue.Name.equals(PropertyName))
                    {
                        return aValue.Value;
                    }
                }
            }
        }
        return null;
    }

    public static Object getPropertyValuefromAny(Object[] CurPropertyValue, String PropertyName, java.lang.Class<?> xClass)
    {
        try
        {
            if (CurPropertyValue != null)
            {
                int MaxCount = CurPropertyValue.length;
                for (int i = 0; i < MaxCount; i++)
                {
                    if (CurPropertyValue[i] != null)
                    {
                        PropertyValue aValue = (PropertyValue) CurPropertyValue[i];
                        if (aValue != null && aValue.Name.equals(PropertyName))
                        {
                            return com.sun.star.uno.AnyConverter.toObject(new com.sun.star.uno.Type(xClass), aValue.Value);
                        }
                    }
                }
            }
            return null;
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
            return null;
        }
    }

    public static Object getUnoPropertyValue(Object oUnoObject, String PropertyName)
    {
        try
        {
            if (oUnoObject != null)
            {
                XPropertySet xPSet = UnoRuntime.queryInterface(XPropertySet.class, oUnoObject);
                return xPSet.getPropertyValue(PropertyName);
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
        return null;
    }

    public static Object getUnoArrayPropertyValue(Object oUnoObject, String PropertyName)
    {
        try
        {
            if (oUnoObject != null)
            {
                XPropertySet xPSet = UnoRuntime.queryInterface(XPropertySet.class, oUnoObject);
                Object oObject = xPSet.getPropertyValue(PropertyName);
                if (AnyConverter.isArray(oObject))
                {
                    return getArrayValue(oObject);
                }
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
        return null;
    }

    public static Object getUnoStructValue(Object oUnoObject, String PropertyName)
    {
        try
        {
            if (oUnoObject != null)
            {
                XPropertySet xPSet = UnoRuntime.queryInterface(XPropertySet.class, oUnoObject);
                if (xPSet.getPropertySetInfo().hasPropertyByName(PropertyName))
                {
                    return xPSet.getPropertyValue(PropertyName);
                }
            }
            return null;
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
            return null;
        }
    }

    public static void setUnoPropertyValues(Object oUnoObject, String[] PropertyNames, Object[] PropertyValues)
    {
        try
        {
            com.sun.star.beans.XMultiPropertySet xMultiPSetLst = UnoRuntime.queryInterface(com.sun.star.beans.XMultiPropertySet.class, oUnoObject);
            if (xMultiPSetLst != null)
            {
                xMultiPSetLst.setPropertyValues(PropertyNames, PropertyValues);
            }
            else
            {
                for (int i = 0; i < PropertyNames.length; i++)
                {
                    setUnoPropertyValue(oUnoObject, PropertyNames[i], PropertyValues[i]);
                }
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
        }
    }

    /**
     * checks if the value of an object that represents an array is null.
     * check beforehand if the Object is really an array with "AnyConverter.IsArray(oObject)
     * @param oValue the paramter that has to represent an object
     * @return a null reference if the array is empty
     */
    public static Object getArrayValue(Object oValue)
    {
        try
        {
            Object oPropList = com.sun.star.uno.AnyConverter.toArray(oValue);
            int nlen = java.lang.reflect.Array.getLength(oPropList);
            if (nlen == 0)
            {
                return null;
            }
            else
            {
                return oPropList;
            }
        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.err);
            return null;
        }
    }
    private static long DAY_IN_MILLIS = (24 * 60 * 60 * 1000);

    public static class DateUtils
    {

        private long docNullTime;
        private XNumberFormatter formatter;
        private XNumberFormatsSupplier formatSupplier;
        private Calendar calendar;

        public DateUtils(XMultiServiceFactory xmsf, Object document) throws Exception
        {
            XMultiServiceFactory docMSF = UnoRuntime.queryInterface(XMultiServiceFactory.class, document);

            Object defaults = docMSF.createInstance("com.sun.star.text.Defaults");
            Locale l = (Locale) Helper.getUnoStructValue(defaults, "CharLocale");

            java.util.Locale jl = new java.util.Locale(
                    l.Language, l.Country, l.Variant);

            calendar = Calendar.getInstance(jl);

            formatSupplier = UnoRuntime.queryInterface(XNumberFormatsSupplier.class, document);

            Object formatSettings = formatSupplier.getNumberFormatSettings();
            com.sun.star.util.Date date = (com.sun.star.util.Date) Helper.getUnoPropertyValue(formatSettings, "NullDate");

            calendar.set(date.Year, date.Month - 1, date.Day);
            docNullTime = getTimeInMillis();

            formatter = NumberFormatter.createNumberFormatter(xmsf, formatSupplier);
        }

        /**
         * @param format a constant of the enumeration NumberFormatIndex
         * @return
         */
        public int getFormat(short format)
        {
            return NumberFormatter.getNumberFormatterKey(formatSupplier, format);
        }

        public XNumberFormatter getFormatter()
        {
            return formatter;
        }

        private long getTimeInMillis()
        {
            java.util.Date dDate = calendar.getTime();
            return dDate.getTime();
        }

        /**
         * @param date a VCL date in form of 20041231
         * @return a document relative date
         */
        public synchronized double getDocumentDateAsDouble(int date)
        {
            calendar.clear();
            calendar.set(date / 10000,
                    (date % 10000) / 100 - 1,
                    date % 100);

            long date1 = getTimeInMillis();
            /*
             * docNullTime and date1 are in millis, but
             * I need a day...
             */
            return (date1 - docNullTime) / DAY_IN_MILLIS + 1;
        }

        public double getDocumentDateAsDouble(DateTime date)
        {
            return getDocumentDateAsDouble(date.Year * 10000 + date.Month * 100 + date.Day);
        }

        public synchronized double getDocumentDateAsDouble(long javaTimeInMillis)
        {
            calendar.clear();
            JavaTools.setTimeInMillis(calendar, javaTimeInMillis);

            long date1 = getTimeInMillis();

            /*
             * docNullTime and date1 are in millis, but
             * I need a day...
             */
            return (date1 - docNullTime) / DAY_IN_MILLIS + 1;

        }

        public String format(int formatIndex, int date)
        {
            return formatter.convertNumberToString(formatIndex, getDocumentDateAsDouble(date));
        }

        public String format(int formatIndex, DateTime date)
        {
            return formatter.convertNumberToString(formatIndex, getDocumentDateAsDouble(date));
        }

        public String format(int formatIndex, long javaTimeInMillis)
        {
            return formatter.convertNumberToString(formatIndex, getDocumentDateAsDouble(javaTimeInMillis));
        }
    }

    public static XComponentContext getComponentContext(XMultiServiceFactory _xMSF)
    {
        // Get the path to the extension and try to add the path to the class loader
        final XPropertySet xProps = UnoRuntime.queryInterface(XPropertySet.class, _xMSF);
        final PropertySetHelper aHelper = new PropertySetHelper(xProps);
        final Object aDefaultContext = aHelper.getPropertyValueAsObject("DefaultContext");
        return UnoRuntime.queryInterface(XComponentContext.class, aDefaultContext);
    }

    public static XMacroExpander getMacroExpander(XMultiServiceFactory _xMSF)
    {
        final XComponentContext xComponentContext = getComponentContext(_xMSF);
        final Object aSingleton = xComponentContext.getValueByName("/singletons/com.sun.star.util.theMacroExpander");
        return UnoRuntime.queryInterface(XMacroExpander.class, aSingleton);
    }
}
