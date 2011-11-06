/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
