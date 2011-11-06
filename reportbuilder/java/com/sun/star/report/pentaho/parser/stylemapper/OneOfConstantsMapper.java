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


package com.sun.star.report.pentaho.parser.stylemapper;

import com.sun.star.report.pentaho.parser.StyleMapper;

import java.util.HashMap;
import java.util.Map;

import org.jfree.layouting.input.style.CSSDeclarationRule;
import org.jfree.layouting.input.style.StyleKey;
import org.jfree.layouting.input.style.values.CSSValue;


public abstract class OneOfConstantsMapper implements StyleMapper
{

    private final StyleKey styleKey;
    private final Map mappings;

    protected OneOfConstantsMapper(final StyleKey styleKey)
    {
        this.styleKey = styleKey;
        this.mappings = new HashMap();
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
        return (CSSValue) mappings.get(attrValue);
    }
}
