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

import org.jfree.layouting.input.style.keys.text.TextDecorationStyle;
import org.jfree.layouting.input.style.keys.text.TextStyleKeys;

public class TextUnderlineStyleMapper extends OneOfConstantsMapper
{

    public TextUnderlineStyleMapper()
    {
        super(TextStyleKeys.TEXT_UNDERLINE_STYLE);
        addMapping("none", TextDecorationStyle.NONE);
        addMapping("solid", TextDecorationStyle.SOLID);
        addMapping("dotted", TextDecorationStyle.DOTTED);
        addMapping("dash", TextDecorationStyle.DASHED);
        addMapping("long-dash", TextDecorationStyle.LONG_DASH);
        addMapping("dot-dash", TextDecorationStyle.DOT_DASH);
        addMapping("dot-dot-dash", TextDecorationStyle.DOT_DOT_DASH);
        addMapping("wave", TextDecorationStyle.WAVE);
    }
}
