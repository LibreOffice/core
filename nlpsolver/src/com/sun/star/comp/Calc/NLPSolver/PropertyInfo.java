/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2009 by Sun Microsystems, Inc.
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

package com.sun.star.comp.Calc.NLPSolver;

import com.sun.star.beans.Property;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.uno.Type;

public class PropertyInfo<PropType> {

    private Property m_property;
    private PropType m_value;
    private String m_description;

    public Property getProperty() {
        return m_property;
    }

    public PropType getValue() {
        return m_value;
    }

    public String getDescription() {
        return m_description;
    }

    public void setValue(Object value) throws IllegalArgumentException {
        if (m_property.Type == Type.LONG) {
            if (!(value instanceof Integer))
                throw new IllegalArgumentException();
            m_value = (PropType)value;
        } else if (m_property.Type == Type.DOUBLE) {
            if (!(value instanceof Double))
                throw new IllegalArgumentException();
            m_value = (PropType)value;
        } else if (m_property.Type == Type.BOOLEAN) {
            if (!(value instanceof Boolean))
                throw new IllegalArgumentException();
            m_value = (PropType)value;
        }
    }

    public PropertyInfo(String name, PropType value, String description) {
        this(name, value, (short)0, description);
    }

    public PropertyInfo(String name, PropType value, short attributes,
            String description) {
        m_property = new Property();
        m_property.Name = name;
        m_property.Attributes = attributes;
        m_property.Handle = -1;

        if (value instanceof Integer)
            m_property.Type = Type.LONG;
        else if (value instanceof Double)
            m_property.Type = Type.DOUBLE;
        else if (value instanceof Boolean)
            m_property.Type = Type.BOOLEAN;

        m_value = value;
        m_description = description;
    }

    public void localize(ResourceManager resourceManager) {
        try {
            m_description = resourceManager.getLocalizedString("Properties." + m_property.Name);
	    System.out.println("Localised description to " + m_description);
        } catch (com.sun.star.resource.MissingResourceException ex) {
	    System.out.println("No properties file !");
        }
    }

}
