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
import com.sun.star.report.XFormattedField;

public class SectionTextField extends SectionObject
{

    private SectionTextField(XFormattedField _aFormattedField)
    {
        m_aParentObject = _aFormattedField;
    // We would like to know, what properties are in this object.
    }

    public static SectionObject create(XFormattedField _aFormattedField)
    {
        return new SectionTextField(_aFormattedField);
    }

    @Override
    public FontDescriptor getFontDescriptor()
    {
        FontDescriptor a = null;
        try
        {
            XFormattedField aField = (XFormattedField) getParent();
            a = aField.getFontDescriptor();
        }
        catch (com.sun.star.beans.UnknownPropertyException e)
        {
        }
        return a;
    }
}

