/*************************************************************************
 *
 *  $RCSfile: ConfigGroup.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $  $Date: 2004-05-19 12:35:19 $
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
 */
package com.sun.star.wizards.common;

import java.lang.reflect.Field;

/**
 *
 * @author  rpiterman
 */
public class ConfigGroup implements ConfigNode {

    public Object root;

    public void writeConfiguration(Object configurationView, Object param) {
        Field[] fields = getClass().getFields();
        for (int i = 0; i < fields.length; i++)
            if (fields[i].getName().startsWith((String) param))
                try {
                    writeField(fields[i], configurationView, (String) param);
                } catch (Exception ex) {
                    System.out.println("Error writing field: " + fields[i].getName());
                    ex.printStackTrace();
                }
    }

    private void writeField(Field field, Object configView, String prefix) throws Exception {
        String propertyName = field.getName().substring(prefix.length());
        //System.out.println("Going to save:" + propertyName);
        Class fieldType = field.getType();
        if (ConfigNode.class.isAssignableFrom(fieldType)) {
            Object childView = Configuration.addConfigNode(configView, propertyName);
            ConfigNode child = (ConfigNode) field.get(this);
            child.writeConfiguration(childView, prefix);
        } else if (fieldType.isPrimitive()) {
            Configuration.set(convertValue(field), propertyName, configView);
        } else if (fieldType.equals(String.class))
            Configuration.set(field.get(this), propertyName, configView);
    }
    /**
     * convert the primitive type value of the
     * given Field object to the corresponding
     * Java Object value.
     * @param field
     * @return the value of the field as a Object.
     */
    public Object convertValue(Field field) throws IllegalAccessException {
        if (field.getType().equals(Boolean.TYPE))
            return (field.getBoolean(this) ? Boolean.TRUE : Boolean.FALSE);
        if (field.getType().equals(Integer.TYPE))
            return new Integer(field.getInt(this));
        if (field.getType().equals(Short.TYPE))
            return new Short(field.getShort(this));
        if (field.getType().equals(Float.TYPE))
            return new Double(field.getFloat(this));
        if (field.getType().equals(Double.TYPE)) {
            return new Double(field.getDouble(this));
        }
        System.out.println("ohoh...");
        return null; //and good luck with it :-) ...
    }

    public void readConfiguration(Object configurationView, Object param) {
        Field[] fields = getClass().getFields();
        for (int i = 0; i < fields.length; i++)
            if (fields[i].getName().startsWith((String) param))
                try {
                    readField(fields[i], configurationView, (String) param);
                } catch (Exception ex) {
                    System.out.println("Error reading field: " + fields[i].getName());
                    ex.printStackTrace();
                }
    }

    private void readField(Field field, Object configView, String prefix) throws Exception {
        String propertyName = field.getName().substring(prefix.length());

        Class fieldType = field.getType();
        if (ConfigNode.class.isAssignableFrom(fieldType)) {
            ConfigNode child = (ConfigNode) field.get(this);
            child.setRoot(root);
            child.readConfiguration(Configuration.getNode(propertyName, configView), prefix);
        } else if (fieldType.isPrimitive()) {
            if (fieldType.equals(Boolean.TYPE))
                field.setBoolean(this, Configuration.getBoolean(propertyName, configView));
            else if (fieldType.equals(Integer.TYPE))
                field.setInt(this, Configuration.getInt(propertyName, configView));
            else if (fieldType.equals(Short.TYPE))
                field.setShort(this, Configuration.getShort(propertyName, configView));
            else if (fieldType.equals(Float.TYPE))
                field.setFloat(this, Configuration.getFloat(propertyName, configView));
            else if (fieldType.equals(Double.TYPE))
                field.setDouble(this, Configuration.getDouble(propertyName, configView));
        } else if (fieldType.equals(String.class))
            field.set(this, Configuration.getString(propertyName, configView));
    }

    public void setRoot(Object newRoot) {
        root = newRoot;
    }

    /* (non-Javadoc)
     * @see com.sun.star.wizards.common.ConfigNode#writeConfiguration(java.lang.Object, java.lang.Object)
     */

}
