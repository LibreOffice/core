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
        Class<?> fieldType = field.getType();
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

        Class<?> fieldType = field.getType();
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
