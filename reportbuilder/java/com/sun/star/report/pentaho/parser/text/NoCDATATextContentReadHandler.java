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

/*
 * Copyright (c) 2007, Your Corporation. All Rights Reserved.
 */
package com.sun.star.report.pentaho.parser.text;

import com.sun.star.report.OfficeToken;
import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.pentaho.parser.ElementReadHandler;
import com.sun.star.report.pentaho.parser.rpt.FixedContentReadHandler;
import com.sun.star.report.pentaho.parser.rpt.FormattedTextReadHandler;
import com.sun.star.report.pentaho.parser.rpt.ImageReadHandler;
import com.sun.star.report.pentaho.parser.rpt.SubDocumentReadHandler;

import java.util.ArrayList;
import java.util.List;

import org.jfree.report.structure.Element;
import org.jfree.report.structure.Section;
import org.jfree.report.structure.StaticText;

import org.pentaho.reporting.libraries.xmlns.parser.XmlReadHandler;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;


/**
 * This is a generic implementation that accepts all input and adds special
 * handlers for the report-elements.
 *
 * @author Thomas Morgner
 */
public class NoCDATATextContentReadHandler extends ElementReadHandler
{

    private Section section;
    private List children;
    private boolean copyType;

    public NoCDATATextContentReadHandler(final Section section,
            final boolean copyType)
    {
        this.children = new ArrayList();
        this.section = section;
        this.copyType = copyType;
    }

    public NoCDATATextContentReadHandler(final Section section)
    {
        this(section, false);
    }

    public NoCDATATextContentReadHandler()
    {
        this(new Section(), true);
    }

    /**
     * Starts parsing.
     *
     * @param attrs the attributes.
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
    protected void startParsing(final Attributes attrs) throws SAXException
    {
        super.startParsing(attrs);
        final Element element = getElement();
        if (copyType)
        {
            copyElementType(element);
        }
        copyAttributes(attrs, element);
    }

    /**
     * Returns the handler for a child element.
     *
     * @param tagName the tag name.
     * @param atts    the attributes.
     * @return the handler or null, if the tagname is invalid.
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
    protected XmlReadHandler getHandlerForChild(final String uri,
            final String tagName,
            final Attributes atts)
            throws SAXException
    {
        if (OfficeNamespaces.OOREPORT_NS.equals(uri))
        {
            if ("fixed-content".equals(tagName))
            {
                final FixedContentReadHandler fixedContentReadHandler = new FixedContentReadHandler();
                children.add(fixedContentReadHandler);
                return fixedContentReadHandler;
            }
            if ("formatted-text".equals(tagName))
            {
                final FormattedTextReadHandler formattedTextReadHandler = new FormattedTextReadHandler();
                children.add(formattedTextReadHandler);
                return formattedTextReadHandler;
            }
            if (OfficeToken.IMAGE.equals(tagName))
            {
                final ImageReadHandler imageReadHandler = new ImageReadHandler();
                children.add(imageReadHandler);
                return imageReadHandler;
            }
            if ("sub-document".equals(tagName))
            {
                final SubDocumentReadHandler subDocReadHandler = new SubDocumentReadHandler(section);
                // children.add(subDocReadHandler);
                return subDocReadHandler;
            }
        }
        if (OfficeNamespaces.DRAWING_NS.equals(uri))
        {
            final XmlReadHandler readHandler;
            if (OfficeToken.IMAGE.equals(tagName))
            {
                readHandler = new ImageReadHandler();
            }
            else
            {
                readHandler = new NoCDATATextContentReadHandler();
            }
            children.add(readHandler);
            return readHandler;
        }
        else
        {
            final TextContentReadHandler readHandler = new TextContentReadHandler();
            children.add(readHandler);
            return readHandler;
        }
    }

    public List getChildren()
    {
        return children;
    }

    /**
     * Done parsing.
     *
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
    protected void doneParsing() throws SAXException
    {
        for (int i = 0; i < children.size(); i++)
        {
            final Object o = children.get(i);
            if (o instanceof ElementReadHandler)
            {
                final ElementReadHandler handler = (ElementReadHandler) o;
                section.addNode(handler.getElement());
            }
            else if (o instanceof StaticText)
            {
                section.addNode((StaticText) o);
            }
        }
    }

    public Element getElement()
    {
        return section;
    }
}
