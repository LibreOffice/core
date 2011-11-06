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


package com.sun.star.report.pentaho.parser.stylemapper.fo;

import com.sun.star.report.pentaho.parser.stylemapper.OneOfConstantsMapper;

import org.jfree.layouting.input.style.keys.text.TextAlign;
import org.jfree.layouting.input.style.keys.text.TextStyleKeys;

public class TextAlignMapper extends OneOfConstantsMapper
{

    public TextAlignMapper()
    {
        super(TextStyleKeys.TEXT_ALIGN);
        addMapping("start", TextAlign.START);
        addMapping("end", TextAlign.END);
        addMapping("left", TextAlign.LEFT);
        addMapping("center", TextAlign.CENTER);
        addMapping("right", TextAlign.RIGHT);
        addMapping("justify", TextAlign.JUSTIFY);
    }
}
