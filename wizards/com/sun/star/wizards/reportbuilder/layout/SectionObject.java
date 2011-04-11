/*
 * ***********************************************************************
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
 * **********************************************************************
 */
package com.sun.star.wizards.reportbuilder.layout;

import com.sun.star.awt.FontDescriptor;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.common.PropertySetHelper;

/**
 *
 * @author ll93751
 */
abstract public class SectionObject
{

    Object m_aParentObject; // this could be FixedText or FormattedField or null
    PropertySetHelper m_aPropertySetHelper;

    public SectionObject()
    {
        m_aParentObject = null;
    }

    protected Object getParent()
    {
        return m_aParentObject;
    }

    abstract public FontDescriptor getFontDescriptor();

    private PropertySetHelper getPropertySetHelper()
    {
        if (m_aPropertySetHelper == null)
        {
            m_aPropertySetHelper = new PropertySetHelper(getParent());
        }
        return m_aPropertySetHelper;
    }

    public int getHeight(int _nDefault)
    {
        final int nHeight = getPropertySetHelper().getPropertyValueAsInteger(PropertyNames.PROPERTY_HEIGHT, _nDefault);
        return nHeight;
    }

    public float getCharWeight(float _nDefault)
    {
        final float fCharWeight = (float) getPropertySetHelper().getPropertyValueAsDouble("CharWeight", _nDefault);
        return fCharWeight;
    }

    public void setFontToBold()
    {
        setPropertyValue("CharWeight", new Float(com.sun.star.awt.FontWeight.BOLD));
    }

    public void setPropertyValue(String _sKey, Object _nValue)
    {
        getPropertySetHelper().setPropertyValueDontThrow(_sKey, _nValue);
    }
}
