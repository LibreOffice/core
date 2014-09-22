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
package com.sun.star.wizards.reportbuilder.layout;

import com.sun.star.awt.FontDescriptor;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.common.PropertySetHelper;

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
        return getPropertySetHelper().getPropertyValueAsInteger(PropertyNames.PROPERTY_HEIGHT, _nDefault);
    }

    public float getCharWeight(float _nDefault)
    {
        return(float) getPropertySetHelper().getPropertyValueAsDouble("CharWeight", _nDefault);
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
