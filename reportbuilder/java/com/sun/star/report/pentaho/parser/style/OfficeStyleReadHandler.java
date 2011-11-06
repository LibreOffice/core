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

import com.sun.star.report.pentaho.model.OfficeStyle;
import com.sun.star.report.pentaho.parser.ElementReadHandler;

import java.util.ArrayList;
import java.util.List;

import org.jfree.report.structure.Element;

import org.pentaho.reporting.libraries.xmlns.parser.XmlReadHandler;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;


/**
 * This class reads a single style rule. The resulting 'office-style' element
 * is added to an 'office-styles' set.
 */
public class OfficeStyleReadHandler extends ElementReadHandler
{

    private final OfficeStyle officeStyle;
    private final List childs;

    public OfficeStyleReadHandler()
    {
        this.officeStyle = new OfficeStyle();
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
//    if (OfficeParserUtil.getInstance().isValidStyleElement(uri, tagName))
//    {
//    }
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
    protected void doneParsing() throws SAXException
    {
        for (int i = 0; i < childs.size(); i++)
        {
            final ElementReadHandler handler = (ElementReadHandler) childs.get(i);
            officeStyle.addNode(handler.getElement());
        }
    }

    public OfficeStyle getOfficeStyle()
    {
        return officeStyle;
    }

    public Element getElement()
    {
        return officeStyle;
    }
}
