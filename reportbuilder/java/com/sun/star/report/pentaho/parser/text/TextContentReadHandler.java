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


package com.sun.star.report.pentaho.parser.text;

import org.jfree.report.structure.Section;
import org.jfree.report.structure.StaticText;

import org.xml.sax.SAXException;

/**
 * This is a generic implementation that accepts all input and adds special
 * handlers for the report-elements.
 *
 * @author Thomas Morgner
 */
public class TextContentReadHandler extends NoCDATATextContentReadHandler
{

    public TextContentReadHandler(final Section section, final boolean copyType)
    {
        super(section, copyType);
    }

    public TextContentReadHandler(final Section section)
    {
        super(section);
    }

    public TextContentReadHandler()
    {
    }

    /**
     * This method is called to process the character data between element tags.
     *
     * @param ch     the character buffer.
     * @param start  the start index.
     * @param length the length.
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
    public void characters(final char[] ch, final int start, final int length)
            throws SAXException
    {
        getChildren().add(new StaticText(new String(ch, start, length)));
    }
}
