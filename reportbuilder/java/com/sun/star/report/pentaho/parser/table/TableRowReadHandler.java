/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: TableRowReadHandler.java,v $
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
 * Creation-Date: 03.07.2006, 13:51:47
 *
 * @author Thomas Morgner
 */
public class TableRowReadHandler extends ElementReadHandler
{

    private final List tableCells;
    private final Section tableRow;

    public TableRowReadHandler()
    {
        tableCells = new ArrayList();
        tableRow = new Section();
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
        final XmlReadHandler rh;
        if (OfficeNamespaces.TABLE_NS.equals(uri))
        {
            if (OfficeToken.TABLE_CELL.equals(tagName))
            {
                rh = new TableCellReadHandler();
            }
            else if (OfficeToken.COVERED_TABLE_CELL.equals(tagName))
            {
                rh = new CoveredCellReadHandler();
            }
            else
            {
                rh = null;
            }
            if (rh != null)
            {
                tableCells.add(rh);
            }
        }
        else
        {
            rh = null;
        }
        return rh;
    }

    /**
     * Done parsing.
     *
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
    protected void doneParsing() throws SAXException
    {
        for (int i = 0; i < tableCells.size(); i++)
        {
            final ElementReadHandler handler = (ElementReadHandler) tableCells.get(i);
            tableRow.addNode(handler.getElement());
        }
    }

    /**
     * Returns the object for this element or null, if this element does not
     * create an object.
     *
     * @return the object.
     */
    public Element getElement()
    {
        return tableRow;
    }
}
