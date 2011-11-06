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


package com.sun.star.report.pentaho.parser.style;

import com.sun.star.report.pentaho.model.FontFaceElement;
import com.sun.star.report.pentaho.parser.ElementReadHandler;

import org.jfree.report.structure.Element;

/**
 * Reads a single font-face declaration. This is equal to the @font rule of
 * the CSS standard. We do not interpret the attributes of the element in
 * any way yet.
 *
 * @author Thomas Morgner
 * @since 13.03.2007
 */
public class FontFaceReadHandler extends ElementReadHandler
{

    private final FontFaceElement fontFaceElement;

    public FontFaceReadHandler()
    {
        this.fontFaceElement = new FontFaceElement();
    }

    public Element getElement()
    {
        return fontFaceElement;
    }
}
