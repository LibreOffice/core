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


package com.sun.star.report.pentaho.parser.table;

import com.sun.star.report.OfficeToken;
import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.pentaho.model.OfficeTableSection;
import com.sun.star.report.pentaho.parser.ElementReadHandler;
import com.sun.star.report.pentaho.parser.rpt.ConditionalPrintExpressionReadHandler;

import java.util.ArrayList;
import java.util.List;

import org.jfree.report.structure.Element;
import org.jfree.report.structure.Section;

import org.pentaho.reporting.libraries.xmlns.parser.XmlReadHandler;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;


/**
 * Creation-Date: 03.07.2006, 13:47:47
 *
 * @author Thomas Morgner
 */
public class TableReadHandler extends ElementReadHandler
{

    private final List children;
    private final Section table;

    public TableReadHandler()
    {
        children = new ArrayList();
        table = new OfficeTableSection();
    }

    /**
     * Starts parsing.
     *
     * @param attrs the attributes.
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
    protected void startParsing(final Attributes attrs)
            throws SAXException
    {
        super.startParsing(attrs);
        final String enabled = attrs.getValue(OfficeNamespaces.OOREPORT_NS, "visible");
        if (enabled == null || OfficeToken.TRUE.equals(enabled))
        {
            table.setEnabled(true);
        }
        else
        {
            table.setEnabled(false);
        }

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
        if (OfficeNamespaces.OOREPORT_NS.equals(uri) && "conditional-print-expression".equals(tagName))
        {
            return new ConditionalPrintExpressionReadHandler(table);
        }
        else if (OfficeNamespaces.TABLE_NS.equals(uri))
        {
            if (OfficeToken.TABLE_COLUMNS.equals(tagName) || OfficeToken.TABLE_HEADER_COLUMNS.equals(tagName))
            {
                final TableColumnsReadHandler columns = new TableColumnsReadHandler();
                children.add(columns);
                return columns;
            }
            else if (OfficeToken.TABLE_ROW.equals(tagName))
            {
                final TableRowReadHandler rowHandler = new TableRowReadHandler();
                children.add(rowHandler);
                return rowHandler;
            }
            else if (OfficeToken.TABLE_ROWS.equals(tagName) || OfficeToken.TABLE_HEADER_ROWS.equals(tagName))
            {
                final TableRowsReadHandler rowsHandler = new TableRowsReadHandler();
                children.add(rowsHandler);
                return rowsHandler;
            }
        }
        return null;
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
            final ElementReadHandler handler = (ElementReadHandler) children.get(i);
            table.addNode(handler.getElement());
        }
    }

    public Element getElement()
    {
        return table;
    }
}
