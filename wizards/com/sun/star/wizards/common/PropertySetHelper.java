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

import com.sun.star.uno.UnoRuntime;
import com.sun.star.beans.XPropertySet;
import com.sun.star.uno.AnyConverter;
import java.util.HashMap;

public class PropertySetHelper
{

    private XPropertySet m_xPropertySet;
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
     */
    private void setPropertyValue(String _sName, Object _aValue) throws java.lang.Exception
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
            catch (com.sun.star.lang.WrappedTargetException e)
            {
                DebugHelper.writeInfo(e.getMessage());
                DebugHelper.exception(e);
            }
        }
        else
        {
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
        if (aObject == null && getHashMap().containsKey(_sName))
        {
            aObject = getHashMap().get(_sName);
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

}
