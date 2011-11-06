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


package com.sun.star.report.pentaho.parser.stylemapper.style;

import com.sun.star.report.pentaho.parser.stylemapper.OneOfConstantsMapper;

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
