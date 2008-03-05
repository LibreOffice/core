/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TableReadHandler.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:47:46 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *    Copyright 2007 by Pentaho Corporation
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
package com.sun.star.report.pentaho.parser.table;

import java.util.ArrayList;

import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.pentaho.model.OfficeTableSection;
import com.sun.star.report.pentaho.parser.ElementReadHandler;
import com.sun.star.report.pentaho.parser.rpt.ConditionalPrintExpressionReadHandler;
import org.jfree.report.structure.Element;
import org.jfree.report.structure.Section;
import org.jfree.xmlns.parser.XmlReadHandler;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

/**
 * Creation-Date: 03.07.2006, 13:47:47
 *
 * @author Thomas Morgner
 */
public class TableReadHandler extends ElementReadHandler
{

    private TableColumnsReadHandler columns;
    private ArrayList tableRows;
    private Section table;

    public TableReadHandler()
    {
        tableRows = new ArrayList();
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
        if (enabled == null || "true".equals(enabled))
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
        if (OfficeNamespaces.OOREPORT_NS.equals(uri))
        {
            if ("conditional-print-expression".equals(tagName))
            {
                return new ConditionalPrintExpressionReadHandler(table);
            }
            return null;
        }

        if (OfficeNamespaces.TABLE_NS.equals(uri) == false)
        {
            return null;
        }
        if ("table-columns".equals(tagName) || "table-header-columns".equals(tagName))
        {
            columns = new TableColumnsReadHandler();
            return columns;
        }
        if ("table-row".equals(tagName))
        {
            final TableRowReadHandler rowHandler = new TableRowReadHandler();
            tableRows.add(rowHandler);
            return rowHandler;
        }
        if ("table-rows".equals(tagName) || "table-header-rows".equals(tagName))
        {
            final TableRowsReadHandler rowsHandler = new TableRowsReadHandler();
            tableRows.add(rowsHandler);
            return rowsHandler;
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
        if (columns != null)
        {
            table.addNode(columns.getElement());
        }

        for (int i = 0; i < tableRows.size(); i++)
        {
            final TableRowReadHandler handler = (TableRowReadHandler) tableRows.get(i);
            table.addNode(handler.getElement());
        }
    }

    public Element getElement()
    {
        return table;
    }
}
