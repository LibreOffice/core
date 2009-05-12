/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ChartReadHandler.java,v $
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
package com.sun.star.report.pentaho.parser.chart;

import com.sun.star.report.pentaho.OfficeNamespaces;
import java.util.ArrayList;

import com.sun.star.report.pentaho.parser.ElementReadHandler;
import com.sun.star.report.pentaho.parser.rpt.DetailRootTableReadHandler;
import com.sun.star.report.pentaho.parser.rpt.ReportReadHandler;
import com.sun.star.report.pentaho.parser.text.TextContentReadHandler;
import java.util.List;
import org.jfree.report.structure.Element;
import org.jfree.report.structure.Section;
import org.pentaho.reporting.libraries.xmlns.parser.XmlReadHandler;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

/**
 *
 * @author Ocke Janssen
 */
public class ChartReadHandler extends ElementReadHandler
{

    private final Section element;
    private final List children;
    private final ReportReadHandler reportHandler;

    public ChartReadHandler(ReportReadHandler reportHandler)
    {
        this.reportHandler = reportHandler;
        children = new ArrayList();
        element = new Section();
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
        if ("detail".equals(tagName))
        {
            final DetailRootTableReadHandler detail = new DetailRootTableReadHandler();
            reportHandler.setDetail(detail);
            return detail;
        }
        else if ("p".equals(tagName) && OfficeNamespaces.TEXT_NS.equals(uri))
        {
            final TextContentReadHandler readHandler = new TextContentReadHandler();
            children.add(readHandler);
            return readHandler;
        }
        final ChartReadHandler erh = new ChartReadHandler(reportHandler);
        children.add(erh);
        return erh;
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
            element.addNode(handler.getElement());
        }
    }

    public Element getElement()
    {
        return element;
    }
}
