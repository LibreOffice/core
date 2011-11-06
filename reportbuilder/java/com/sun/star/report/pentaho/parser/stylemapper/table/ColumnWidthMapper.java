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


package com.sun.star.report.pentaho.parser.stylemapper.table;

import com.sun.star.report.pentaho.parser.StyleMapper;

import org.jfree.layouting.input.style.CSSDeclarationRule;
import org.jfree.layouting.input.style.keys.box.BoxStyleKeys;

/**
 * Creation-Date: 03.07.2006, 13:08:27
 *
 * @author Thomas Morgner
 */
public class ColumnWidthMapper implements StyleMapper
{

    public ColumnWidthMapper()
    {
    }

    public void updateStyle(final String uri,
            final String attrName,
            final String attrValue,
            final CSSDeclarationRule targetRule)
    {
        targetRule.setPropertyValueAsString(BoxStyleKeys.WIDTH, attrValue);
    }
}
