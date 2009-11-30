/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: TableColumnsReadHandler.java,v $
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

import org.jfree.report.structure.Section;
import org.jfree.report.structure.Element;
import org.pentaho.reporting.libraries.xmlns.parser.XmlReadHandler;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import com.sun.star.report.pentaho.parser.ElementReadHandler;
import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.OfficeToken;
import java.util.List;

/**
 * Creation-Date: 03.07.2006, 13:50:41
 *
 * @author Thomas Morgner
 */
public class TableColumnsReadHandler extends ElementReadHandler
{

    private final List columns;
    private final Section tableColumns;

    public TableColumnsReadHandler()
    {
        columns = new ArrayList();
        tableColumns = new Section();
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
        if (OfficeNamespaces.TABLE_NS.equals(uri) && OfficeToken.TABLE_COLUMN.equals(tagName) )
        {
            final TableColumnReadHandler readHandler = new TableColumnReadHandler();
            columns.add(readHandler);
            return readHandler;
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
        for (int i = 0; i < columns.size(); i++)
        {
            final TableColumnReadHandler handler = (TableColumnReadHandler) columns.get(i);
            tableColumns.addNode(handler.getElement());
        }
    }

    public Element getElement()
    {
        return tableColumns;
    }
}
