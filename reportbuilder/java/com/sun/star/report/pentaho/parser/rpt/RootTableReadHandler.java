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


package com.sun.star.report.pentaho.parser.rpt;

import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.pentaho.parser.ElementReadHandler;
import com.sun.star.report.pentaho.parser.table.TableReadHandler;

import org.jfree.report.structure.Element;
import org.jfree.report.structure.Section;

import org.pentaho.reporting.libraries.xmlns.parser.XmlReadHandler;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

public class RootTableReadHandler extends ElementReadHandler
{

    private TableReadHandler sectionTableReadHandler;
    private final Section section;

    public RootTableReadHandler()
    {
        section = new Section();
    }

    protected RootTableReadHandler(final Section section)
    {
        if (section == null)
        {
            throw new NullPointerException();
        }
        this.section = section;
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
        if (OfficeNamespaces.TABLE_NS.equals(uri) && "table".equals(tagName))
        {
            sectionTableReadHandler = new TableReadHandler();
            return sectionTableReadHandler;
        }
        if (OfficeNamespaces.OOREPORT_NS.equals(uri) && "conditional-print-expression".equals(tagName))
        {
            return new ConditionalPrintExpressionReadHandler(section);
        }
        return null;
    }

    /**
     * Done parsing.
     *
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
    protected void doneParsing()
            throws SAXException
    {
        if (sectionTableReadHandler != null)
        {
            section.addNode(sectionTableReadHandler.getElement());
        }
    }

    public Element getElement()
    {
        return section;
    }
}
