/*************************************************************************
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
package com.sun.star.wizards.table;

import java.util.Vector;

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
//  String fieldname;
    private String keyname;
    private XNameAccess xNameAccessTableNode;
    private XPropertySet xPropertySet;
    private Vector aPropertyValues;
//  PropertyValue[] aPropertyValues;
    private Integer Type;
    private Integer Scale;
    private Integer Precision;
    private Boolean DefaultValue;
    private String Name;
    private XMultiServiceFactory xMSF;
    private Locale aLocale;

    public FieldDescription(XMultiServiceFactory _xMSF, Locale _aLocale, ScenarioSelector _curscenarioselector, String _fieldname, String _keyname, int _nmaxcharCount)
    {
        xMSF = _xMSF;
        aLocale = _aLocale;
        tablename = _curscenarioselector.getTableName();
        Name = _fieldname;
        keyname = _keyname;
        aPropertyValues = new Vector();
        xNameAccessTableNode = _curscenarioselector.oCGTable.xNameAccessFieldsNode;
        XNameAccess xNameAccessFieldNode;
        if (_curscenarioselector.bcolumnnameislimited)
        {
            xNameAccessFieldNode = Configuration.getChildNodebyDisplayName(xMSF, aLocale, xNameAccessTableNode, keyname, "ShortName", _nmaxcharCount);
        }
        else
        {
            xNameAccessFieldNode = Configuration.getChildNodebyDisplayName(xMSF, aLocale, xNameAccessTableNode, keyname, PropertyNames.PROPERTY_NAME, _nmaxcharCount);
        }
        setFieldProperties(xNameAccessFieldNode);
    }

    public FieldDescription(String _fieldname)
    {
        Name = _fieldname;
        aPropertyValues = new Vector();
        Type = new Integer(com.sun.star.sdbc.DataType.VARCHAR);
        aPropertyValues.addElement(Properties.createProperty(PropertyNames.PROPERTY_NAME, _fieldname));
        aPropertyValues.addElement(Properties.createProperty("Type", Type));
    }

    public void setName(String _newfieldname)
    {
        for (int i = 0; i < aPropertyValues.size(); i++)
        {
            PropertyValue aPropertyValue = (PropertyValue) aPropertyValues.get(i);
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
            e.printStackTrace(System.out);
        }
        return bexists;
    }

    public void setFieldProperties(XNameAccess _xNameAccessFieldNode)
    {
        try
        {
            xPropertySet = UnoRuntime.queryInterface(XPropertySet.class, _xNameAccessFieldNode);
//      Integer Index = (Integer) xPropertySet.getPropertyValue("Index");
            if (propertyexists(PropertyNames.PROPERTY_NAME))
            {
                aPropertyValues.addElement(Properties.createProperty(PropertyNames.PROPERTY_NAME, Name));
            }
            if (propertyexists("Type"))
            {
                aPropertyValues.addElement(Properties.createProperty("Type", xPropertySet.getPropertyValue("Type")));
            }
            if (propertyexists("Scale"))
            {
                aPropertyValues.addElement(Properties.createProperty("Scale", xPropertySet.getPropertyValue("Scale")));
//          Scale =
            }
            if (propertyexists("Precision"))
            {
                aPropertyValues.addElement(Properties.createProperty("Precision", xPropertySet.getPropertyValue("Precision")));
//          Precision = (Integer) xPropertySet.getPropertyValue("Precision");
            }
            if (propertyexists("DefaultValue"))
            {
                aPropertyValues.addElement(Properties.createProperty("DefaultValue", xPropertySet.getPropertyValue("DefaultValue")));//          DefaultValue = (Boolean) xPropertySet.getPropertyValue("DefaultValue");
            //Type =  4; // TODO wo ist der Fehler?(Integer) xPropertySet.getPropertyValue("Type");
            }
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
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
