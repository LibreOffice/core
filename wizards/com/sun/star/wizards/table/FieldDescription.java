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
package com.sun.star.wizards.table;

import java.util.ArrayList;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.Locale;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.Configuration;
import com.sun.star.wizards.common.Properties;
import com.sun.star.wizards.common.PropertyNames;

public class FieldDescription
{
    private String tablename = PropertyNames.EMPTY_STRING;
    private XPropertySet xPropertySet;
    private final ArrayList<PropertyValue> aPropertyValues;
    private String Name;

    public FieldDescription(XMultiServiceFactory _xMSF, Locale _aLocale, ScenarioSelector _curscenarioselector, String _fieldname, String keyname, int _nmaxcharCount)
    {
        tablename = _curscenarioselector.getTableName();
        Name = _fieldname;
        aPropertyValues = new ArrayList<PropertyValue>();
        XNameAccess xNameAccessTableNode = _curscenarioselector.oCGTable.xNameAccessFieldsNode;
        XNameAccess xNameAccessFieldNode;
        if (_curscenarioselector.bcolumnnameislimited)
        {
            xNameAccessFieldNode = Configuration.getChildNodebyDisplayName(_xMSF, _aLocale, xNameAccessTableNode, keyname, "ShortName", _nmaxcharCount);
        }
        else
        {
            xNameAccessFieldNode = Configuration.getChildNodebyDisplayName(_xMSF, _aLocale, xNameAccessTableNode, keyname, PropertyNames.PROPERTY_NAME, _nmaxcharCount);
        }
        setFieldProperties(xNameAccessFieldNode);
    }

    public FieldDescription(String _fieldname)
    {
        Name = _fieldname;
        aPropertyValues = new ArrayList<PropertyValue>();
        Integer Type = Integer.valueOf(com.sun.star.sdbc.DataType.VARCHAR);
        aPropertyValues.add(Properties.createProperty(PropertyNames.PROPERTY_NAME, _fieldname));
        aPropertyValues.add(Properties.createProperty("Type", Type));
    }

    public void setName(String _newfieldname)
    {
        for (int i = 0; i < aPropertyValues.size(); i++)
        {
            PropertyValue aPropertyValue = aPropertyValues.get(i);
            if (aPropertyValue.Name.equals(PropertyNames.PROPERTY_NAME))
            {
                aPropertyValue.Value = _newfieldname;
                aPropertyValues.set(i, aPropertyValue);
                Name = _newfieldname;
                return;
            }
        }
    }

    public String getName()
    {
        return Name;
    }

    public String gettablename()
    {
        return tablename;
    }

    private boolean propertyexists(String _propertyname)
    {
        boolean bexists = false;
        try
        {
            if (xPropertySet.getPropertySetInfo().hasPropertyByName(_propertyname))
            {
                Object oValue = xPropertySet.getPropertyValue(_propertyname);
                bexists = (!com.sun.star.uno.AnyConverter.isVoid(oValue));
            }
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
        }
        return bexists;
    }

    private void setFieldProperties(XNameAccess _xNameAccessFieldNode)
    {
        try
        {
            xPropertySet = UnoRuntime.queryInterface(XPropertySet.class, _xNameAccessFieldNode);
            if (propertyexists(PropertyNames.PROPERTY_NAME))
            {
                aPropertyValues.add(Properties.createProperty(PropertyNames.PROPERTY_NAME, Name));
            }
            if (propertyexists("Type"))
            {
                aPropertyValues.add(Properties.createProperty("Type", xPropertySet.getPropertyValue("Type")));
            }
            if (propertyexists("Scale"))
            {
                aPropertyValues.add(Properties.createProperty("Scale", xPropertySet.getPropertyValue("Scale")));
            }
            if (propertyexists("Precision"))
            {
                aPropertyValues.add(Properties.createProperty("Precision", xPropertySet.getPropertyValue("Precision")));
            }
            if (propertyexists("DefaultValue"))
            {
                aPropertyValues.add(Properties.createProperty("DefaultValue", xPropertySet.getPropertyValue("DefaultValue")));//          DefaultValue = (Boolean) xPropertySet.getPropertyValue("DefaultValue");
            //Type =  4; // TODO wo ist der Fehler?(Integer) xPropertySet.getPropertyValue("Type");
            }
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
        }
    }

    public PropertyValue[] getPropertyValues()
    {
        if (aPropertyValues != null)
        {
            PropertyValue[] aProperties = new PropertyValue[aPropertyValues.size()];
            aPropertyValues.toArray(aProperties);
            return aProperties;
        }
        return null;
    }
}
