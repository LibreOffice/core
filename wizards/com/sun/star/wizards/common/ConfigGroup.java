/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ConfigGroup.java,v $
 * $Revision: 1.7 $
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

import java.lang.reflect.Field;

/**
 *
 * @author  rpiterman
 */
public class ConfigGroup implements ConfigNode
{

    public Object root;

    public void writeConfiguration(Object configurationView, Object param)
    {
        Field[] fields = getClass().getFields();
        for (int i = 0; i < fields.length; i++)
        {
            if (fields[i].getName().startsWith((String) param))
            {
                try
                {
                    writeField(fields[i], configurationView, (String) param);
                }
                catch (Exception ex)
                {
                    System.out.println("Error writing field: " + fields[i].getName());
                    ex.printStackTrace();
                }
            }
        }
    }

    private void writeField(Field field, Object configView, String prefix) throws Exception
    {
        String propertyName = field.getName().substring(prefix.length());
        //System.out.println("Going to save:" + propertyName);
        Class fieldType = field.getType();
        if (ConfigNode.class.isAssignableFrom(fieldType))
        {
            Object childView = Configuration.addConfigNode(configView, propertyName);
            ConfigNode child = (ConfigNode) field.get(this);
            child.writeConfiguration(childView, prefix);
        }
        else if (fieldType.isPrimitive())
        {
            Configuration.set(convertValue(field), propertyName, configView);
        }
        else if (fieldType.equals(String.class))
        {
            Configuration.set(field.get(this), propertyName, configView);
        }
    }

    /**
     * convert the primitive type value of the
     * given Field object to the corresponding
     * Java Object value.
     * @param field
     * @return the value of the field as a Object.
     * @throws IllegalAccessException
     */
    public Object convertValue(Field field) throws IllegalAccessException
    {
        if (field.getType().equals(Boolean.TYPE))
        {
            return (field.getBoolean(this) ? Boolean.TRUE : Boolean.FALSE);
        }
        if (field.getType().equals(Integer.TYPE))
        {
            return new Integer(field.getInt(this));
        }
        if (field.getType().equals(Short.TYPE))
        {
            return new Short(field.getShort(this));
        }
        if (field.getType().equals(Float.TYPE))
        {
            return new Double(field.getFloat(this));
        }
        if (field.getType().equals(Double.TYPE))
        {
            return new Double(field.getDouble(this));
        }
        //System.out.println("ohoh...");
        return null; //and good luck with it :-) ...
    }

    public void readConfiguration(Object configurationView, Object param)
    {
        Field[] fields = getClass().getFields();
        for (int i = 0; i < fields.length; i++)
        {
            if (fields[i].getName().startsWith((String) param))
            {
                try
                {
                    readField(fields[i], configurationView, (String) param);
                }
                catch (Exception ex)
                {
                    System.out.println("Error reading field: " + fields[i].getName());
                    ex.printStackTrace();
                }
            }
        }
    }

    private void readField(Field field, Object configView, String prefix) throws Exception
    {
        String propertyName = field.getName().substring(prefix.length());

        Class fieldType = field.getType();
        if (ConfigNode.class.isAssignableFrom(fieldType))
        {
            ConfigNode child = (ConfigNode) field.get(this);
            child.setRoot(root);
            child.readConfiguration(Configuration.getNode(propertyName, configView), prefix);
        }
        else if (fieldType.isPrimitive())
        {
            if (fieldType.equals(Boolean.TYPE))
            {
                field.setBoolean(this, Configuration.getBoolean(propertyName, configView));
            }
            else if (fieldType.equals(Integer.TYPE))
            {
                field.setInt(this, Configuration.getInt(propertyName, configView));
            }
            else if (fieldType.equals(Short.TYPE))
            {
                field.setShort(this, Configuration.getShort(propertyName, configView));
            }
            else if (fieldType.equals(Float.TYPE))
            {
                field.setFloat(this, Configuration.getFloat(propertyName, configView));
            }
            else if (fieldType.equals(Double.TYPE))
            {
                field.setDouble(this, Configuration.getDouble(propertyName, configView));
            }
        }
        else if (fieldType.equals(String.class))
        {
            field.set(this, Configuration.getString(propertyName, configView));
        }
    }

    public void setRoot(Object newRoot)
    {
        root = newRoot;
    }

    /* (non-Javadoc)
     * @see com.sun.star.wizards.common.ConfigNode#writeConfiguration(java.lang.Object, java.lang.Object)
     */
}
