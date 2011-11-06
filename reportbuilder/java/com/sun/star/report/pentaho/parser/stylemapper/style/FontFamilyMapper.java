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

import com.sun.star.report.pentaho.parser.StyleMapper;

import org.jfree.layouting.input.style.CSSDeclarationRule;
import org.jfree.layouting.input.style.keys.font.FontStyleKeys;
import org.jfree.layouting.input.style.values.CSSStringType;
import org.jfree.layouting.input.style.values.CSSStringValue;
import org.jfree.layouting.input.style.values.CSSValue;
import org.jfree.layouting.input.style.values.CSSValueList;

public class FontFamilyMapper implements StyleMapper
{

    public FontFamilyMapper()
    {
    }

    public void updateStyle(final String uri, final String attrName, final String attrValue,
            final CSSDeclarationRule targetRule)
    {
        final CSSValue value = targetRule.getPropertyCSSValue(FontStyleKeys.FONT_FAMILY);
        if (!(value instanceof CSSValueList))
        {
            final CSSStringValue cssVal = new CSSStringValue(CSSStringType.STRING, attrValue);
            targetRule.setPropertyValue(FontStyleKeys.FONT_FAMILY,
                    new CSSValueList(new CSSValue[]
                    {
                        cssVal
                    }));
        }
        else
        {
            final CSSValueList list = (CSSValueList) value;
            targetRule.setPropertyValue(FontStyleKeys.FONT_FAMILY,
                    CSSValueList.insertFirst(list, value));
        }
    }
}
