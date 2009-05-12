/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: TableReadHandler.java,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
package com.sun.star.report.pentaho.parser.table;

import java.util.ArrayList;

import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.OfficeToken;
import com.sun.star.report.pentaho.model.OfficeTableSection;
import com.sun.star.report.pentaho.parser.ElementReadHandler;
import com.sun.star.report.pentaho.parser.rpt.ConditionalPrintExpressionReadHandler;
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
