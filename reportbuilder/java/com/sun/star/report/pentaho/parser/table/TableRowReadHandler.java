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
import com.sun.star.report.pentaho.parser.ElementReadHandler;

import java.util.ArrayList;
import java.util.List;

import org.jfree.report.structure.Element;
import org.jfree.report.structure.Section;

import org.pentaho.reporting.libraries.xmlns.parser.XmlReadHandler;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;


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
