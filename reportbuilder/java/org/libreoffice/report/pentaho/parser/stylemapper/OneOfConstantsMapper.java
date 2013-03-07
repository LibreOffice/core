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
package org.libreoffice.report.pentaho.parser.stylemapper;

import org.libreoffice.report.pentaho.parser.StyleMapper;

import java.util.HashMap;
import java.util.Map;

import org.jfree.layouting.input.style.CSSDeclarationRule;
import org.jfree.layouting.input.style.StyleKey;
import org.jfree.layouting.input.style.values.CSSValue;


public abstract class OneOfConstantsMapper implements StyleMapper
{

    private final StyleKey styleKey;
    private final Map<String, CSSValue> mappings;

    protected OneOfConstantsMapper(final StyleKey styleKey)
    {
        this.styleKey = styleKey;
        this.mappings = new HashMap<String, CSSValue>();
    }

    public void addMapping(final String value, final CSSValue target)
    {
        mappings.put(value, target);
    }

    public void updateStyle(final String uri,
            final String attrName,
            final String attrValue,
            final CSSDeclarationRule targetRule)
    {
        final CSSValue value = lookupMapping(attrValue);
        if (value != null)
        {
            targetRule.setPropertyValue(styleKey, value);
        }
    }

    public StyleKey getStyleKey()
    {
        return styleKey;
    }

    protected CSSValue lookupMapping(final String attrValue)
    {
        return mappings.get(attrValue);
    }
}
