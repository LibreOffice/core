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
import com.sun.star.report.XFixedText;
// import com.sun.star.wizards.common.PropertySetHelper;
/**
 *
 * @author ll93751
 */
public class SectionLabel extends SectionObject
{

    protected SectionLabel(XFixedText _aFixedText)
    {
        m_aParentObject = _aFixedText;
    // We would like to know, what properties are in this object.
//        PropertySetHelper aHelper = new PropertySetHelper(_aFixedText);
//        aHelper.showProperties();
    }

    public static SectionObject create(XFixedText _aFixedText)
    {
        return new SectionLabel(_aFixedText);
    }

    /**
     * Return the current FontDescriptor
     * @return
     */
    public FontDescriptor getFontDescriptor()
    {
        FontDescriptor a = null;
        try
        {
            final XFixedText aLabel = (XFixedText) getParent();
            a = aLabel.getFontDescriptor();
        }
        catch (com.sun.star.beans.UnknownPropertyException e)
        {
        }
        return a;
    }
}


