/*
 ************************************************************************
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
package com.sun.star.wizards.common;

import com.sun.star.beans.Property;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.uno.AnyConverter;
import com.sun.star.lang.XServiceInfo;

// import com.sun.star.container.XNameAccess;
import java.util.HashMap;

public class PropertySetHelper
{

    protected XPropertySet m_xPropertySet;
    private HashMap<String, Object> m_aHashMap;

    public PropertySetHelper(Object _aObj)
    {
        if (_aObj == null)
        {
            return;
        }
        m_xPropertySet = UnoRuntime.queryInterface(XPropertySet.class, _aObj);
    }

    private HashMap<String, Object> getHashMap()
    {
        if (m_aHashMap == null)
        {
            m_aHashMap = new HashMap<String, Object>();
        }
        return m_aHashMap;
    }

    /**
    set a property, don't throw any exceptions, they will only write down as a hint in the helper debug output
    @param _sName name of the property to set
    @param _aValue property value as object
     */
    public void setPropertyValueDontThrow(String _sName, Object _aValue)
    {
        try
        {
            setPropertyValue(_sName, _aValue);
        }
        catch (Exception e)
        {
            DebugHelper.writeInfo("Don't throw the exception with property name(" + _sName + " ) : " + e.getMessage());
        }
    }

    /**
    set a property,
    @param _sName name of the property to set
    @param _aValue property value as object
     * @throws java.lang.Exception
     */
    public void setPropertyValue(String _sName, Object _aValue) throws java.lang.Exception
    {
        if (m_xPropertySet != null)
        {
            try
            {
                m_xPropertySet.setPropertyValue(_sName, _aValue);
            }
            // Exceptions are not from interest
            catch (com.sun.star.beans.UnknownPropertyException e)
            {
                DebugHelper.writeInfo(e.getMessage());
                DebugHelper.exception(e);
            }
            catch (com.sun.star.beans.PropertyVetoException e)
            {
                DebugHelper.writeInfo(e.getMessage());
                DebugHelper.exception(e);
            }
            catch (com.sun.star.lang.IllegalArgumentException e)
            {
                DebugHelper.writeInfo(e.getMessage());
                DebugHelper.exception(e);
            }
            catch (com.sun.star.lang.WrappedTargetException e)
            {
                DebugHelper.writeInfo(e.getMessage());
                DebugHelper.exception(e);
            }
        }
        else
        {
            // DebugHelper.writeInfo("PropertySetHelper.setProperty() can't get XPropertySet");
            getHashMap().put(_sName, _aValue);
        }
    }

    /**
    get a property and convert it to a int value
    @param _sName the string name of the property
    @param _nDefault if an error occur, return this value
    @return the int value of the property
     */
    public int getPropertyValueAsInteger(String _sName, int _nDefault)
    {
        Object aObject = null;
        int nValue = _nDefault;

        if (m_xPropertySet != null)
        {
            try
            {
                aObject = m_xPropertySet.getPropertyValue(_sName);
            }
            catch (com.sun.star.beans.UnknownPropertyException e)
            {
                DebugHelper.writeInfo(e.getMessage());
            }
            catch (com.sun.star.lang.WrappedTargetException e)
            {
                DebugHelper.writeInfo(e.getMessage());
            }
        }
        if (aObject != null)
        {
            try
            {
                nValue = NumericalHelper.toInt(aObject);
            }
            catch (com.sun.star.lang.IllegalArgumentException e)
            {
                DebugHelper.writeInfo("can't convert a object to integer.");
            }
        }
        return nValue;
    }

    /**
    get a property and convert it to a short value
    @param _sName the string name of the property
    @param _nDefault if an error occur, return this value
    @return the int value of the property
     */
    public short getPropertyValueAsShort(String _sName, short _nDefault)
    {
        Object aObject = null;
        short nValue = _nDefault;

        if (m_xPropertySet != null)
        {
            try
            {
                aObject = m_xPropertySet.getPropertyValue(_sName);
            }
            catch (com.sun.star.beans.UnknownPropertyException e)
            {
                DebugHelper.writeInfo(e.getMessage());
            }
            catch (com.sun.star.lang.WrappedTargetException e)
            {
                DebugHelper.writeInfo(e.getMessage());
            }
        }
        if (aObject != null)
        {
            try
            {
                nValue = NumericalHelper.toShort(aObject);
            }
            catch (com.sun.star.lang.IllegalArgumentException e)
            {
                DebugHelper.writeInfo("can't convert a object to short.");
            }
        }
        return nValue;
    }

    /**
    get a property and convert it to a double value
    @param _sName the string name of the property
    @param _nDefault if an error occur, return this value
    @return the int value of the property
     */
    public double getPropertyValueAsDouble(String _sName, double _nDefault)
    {
        Object aObject = null;
        double nValue = _nDefault;

        if (m_xPropertySet != null)
        {
            try
            {
                aObject = m_xPropertySet.getPropertyValue(_sName);
            }
            catch (com.sun.star.beans.UnknownPropertyException e)
            {
                DebugHelper.writeInfo(e.getMessage());
            }
            catch (com.sun.star.lang.WrappedTargetException e)
            {
                DebugHelper.writeInfo(e.getMessage());
            }
        }
        if (aObject == null)
        {
            if (getHashMap().containsKey(_sName))
            {
                aObject = getHashMap().get(_sName);
            }
        }
        if (aObject != null)
        {
            try
            {
                nValue = NumericalHelper.toDouble(aObject);
            }
            catch (com.sun.star.lang.IllegalArgumentException e)
            {
                DebugHelper.writeInfo("can't convert a object to integer.");
            }
        }
        return nValue;
    }

    /**
    get a property and convert it to a boolean value
    @param _sName the string name of the property
    @param _bDefault if an error occur, return this value
    @return the boolean value of the property
     */
    public boolean getPropertyValueAsBoolean(String _sName, boolean _bDefault)
    {
        Object aObject = null;
        boolean bValue = _bDefault;

        if (m_xPropertySet != null)
        {
            try
            {
                aObject = m_xPropertySet.getPropertyValue(_sName);
            }
            catch (com.sun.star.beans.UnknownPropertyException e)
            {
                DebugHelper.writeInfo(e.getMessage());
                DebugHelper.writeInfo("UnknownPropertyException caught: Name:=" + _sName);
            }
            catch (com.sun.star.lang.WrappedTargetException e)
            {
                DebugHelper.writeInfo(e.getMessage());
            }
        }
        if (aObject != null)
        {
            try
            {
                bValue = NumericalHelper.toBoolean(aObject);
            }
            catch (com.sun.star.lang.IllegalArgumentException e)
            {
                DebugHelper.writeInfo("can't convert a object to boolean.");
            }
        }
        return bValue;
    }

    /**
    get a property and convert it to a string value
    @param _sName the string name of the property
    @param _sDefault if an error occur, return this value
    @return the string value of the property
     */
    public String getPropertyValueAsString(String _sName, String _sDefault)
    {
        Object aObject = null;
        String sValue = _sDefault;

        if (m_xPropertySet != null)
        {
            try
            {
                aObject = m_xPropertySet.getPropertyValue(_sName);
            }
            catch (com.sun.star.beans.UnknownPropertyException e)
            {
                DebugHelper.writeInfo(e.getMessage());
            }
            catch (com.sun.star.lang.WrappedTargetException e)
            {
                DebugHelper.writeInfo(e.getMessage());
            }
        }
        if (aObject != null)
        {
            try
            {
                sValue = AnyConverter.toString(aObject);
            }
            catch (com.sun.star.lang.IllegalArgumentException e)
            {
                DebugHelper.writeInfo("can't convert a object to string.");
            }
        }
        return sValue;
    }

    /**
    get a property and don't convert it
    @param _sName the string name of the property
    @return the object value of the property without any conversion
     */
    public Object getPropertyValueAsObject(String _sName)
    {
        Object aObject = null;

        if (m_xPropertySet != null)
        {
            try
            {
                aObject = m_xPropertySet.getPropertyValue(_sName);
            }
            catch (com.sun.star.beans.UnknownPropertyException e)
            {
                DebugHelper.writeInfo(e.getMessage());
            }
            catch (com.sun.star.lang.WrappedTargetException e)
            {
                DebugHelper.writeInfo(e.getMessage());
            }
        }
        return aObject;
    }

    /**
     * Debug helper, to show all properties which are available in the given object.
     * @param _xObj the object of which the properties should shown
     */
    public static void showProperties(Object _xObj)
    {
        PropertySetHelper aHelper = new PropertySetHelper(_xObj);
        aHelper.showProperties();
    }

    /**
    Debug helper, to show all properties which are available in the current object.
     */
    public void showProperties()
    {
        String sName = PropertyNames.EMPTY_STRING;

        if (m_xPropertySet != null)
        {
            XServiceInfo xServiceInfo = UnoRuntime.queryInterface(XServiceInfo.class, m_xPropertySet);
            if (xServiceInfo != null)
            {
                sName = xServiceInfo.getImplementationName();
            }
            XPropertySetInfo xInfo = m_xPropertySet.getPropertySetInfo();
            Property[] aAllProperties = xInfo.getProperties();
            DebugHelper.writeInfo("Show all properties of Implementation of :'" + sName + "'");
            for (int i = 0; i < aAllProperties.length; i++)
            {
                DebugHelper.writeInfo(" - " + aAllProperties[i].Name);
            }
        }
        else
        {
            DebugHelper.writeInfo("The given object don't support XPropertySet interface.");
        }
    }
}
