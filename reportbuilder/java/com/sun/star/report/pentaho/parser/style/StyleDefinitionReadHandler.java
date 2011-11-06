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

import com.sun.star.report.pentaho.parser.ElementReadHandler;

import java.util.ArrayList;
import java.util.List;

import org.jfree.report.structure.Element;
import org.jfree.report.structure.Section;

import org.pentaho.reporting.libraries.xmlns.parser.XmlReadHandler;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;


/**
 * Reads all childs of a style-definition. This simply copies everything that
 * is contained in the source-file into a generic structure that can be
 * written out later.
 */
public class StyleDefinitionReadHandler extends ElementReadHandler
{

    private final Section rawSection;
    private final List childs;

    public StyleDefinitionReadHandler()
    {
        this.rawSection = new Section();
        this.childs = new ArrayList();
    }

    /**
     * Returns the handler for a child element.
     *
     * @param tagName the tag name.
     * @param atts    the attributes.
     * @return the handler or null, if the tagname is invalid.
     *
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
    protected XmlReadHandler getHandlerForChild(final String uri,
            final String tagName,
            final Attributes atts)
            throws SAXException
    {
        final StyleDefinitionReadHandler readHandler =
                new StyleDefinitionReadHandler();
        childs.add(readHandler);
        return readHandler;
    }

    /**
     * Done parsing.
     *
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
    protected void doneParsing()
            throws SAXException
    {
        for (int i = 0; i < childs.size(); i++)
        {
            final ElementReadHandler handler = (ElementReadHandler) childs.get(i);
            rawSection.addNode(handler.getElement());
        }
    }

    public Element getElement()
    {
        return rawSection;
    }
}
