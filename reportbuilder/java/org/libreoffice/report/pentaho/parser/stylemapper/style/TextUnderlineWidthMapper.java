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
package org.libreoffice.report.pentaho.parser.stylemapper.style;

import org.libreoffice.report.pentaho.parser.stylemapper.OneOfConstantsMapper;

import org.jfree.layouting.input.style.CSSDeclarationRule;
import org.jfree.layouting.input.style.keys.border.BorderWidth;
import org.jfree.layouting.input.style.keys.text.TextDecorationWidth;
import org.jfree.layouting.input.style.keys.text.TextStyleKeys;
import org.jfree.layouting.input.style.values.CSSAutoValue;
import org.jfree.layouting.input.style.values.CSSValue;

public class TextUnderlineWidthMapper extends OneOfConstantsMapper
{

    public TextUnderlineWidthMapper()
    {
        super(TextStyleKeys.TEXT_UNDERLINE_WIDTH);
        addMapping("auto", CSSAutoValue.getInstance());
        addMapping("normal", BorderWidth.MEDIUM);
        addMapping("bold", TextDecorationWidth.BOLD);
        addMapping("thin", BorderWidth.THIN);
        addMapping("dash", TextDecorationWidth.DASH);
        addMapping("medium", BorderWidth.MEDIUM);
        addMapping("thick", BorderWidth.THICK);
    }

    public void updateStyle(final String uri,
            final String attrName,
            final String attrValue,
            final CSSDeclarationRule targetRule)
    {
        if (attrName == null)
        {
            throw new NullPointerException();
        }

        final CSSValue value = lookupMapping(attrValue);
        if (value != null)
        {
            targetRule.setPropertyValue(getStyleKey(), value);
        }
        else
        // percent
        // positive integer
        // positive length
        {
            targetRule.setPropertyValueAsString(getStyleKey(), attrValue);
        }
    }
}
