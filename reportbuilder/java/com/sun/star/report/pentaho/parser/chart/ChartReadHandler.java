/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package com.sun.star.report.pentaho.parser.chart;

import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.pentaho.parser.ElementReadHandler;
import com.sun.star.report.pentaho.parser.rpt.DetailRootTableReadHandler;
import com.sun.star.report.pentaho.parser.rpt.ReportReadHandler;
import com.sun.star.report.pentaho.parser.text.TextContentReadHandler;

import java.util.ArrayList;
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
    private final List<ElementReadHandler> children;
    private final ReportReadHandler reportHandler;

    public ChartReadHandler(ReportReadHandler reportHandler)
    {
        this.reportHandler = reportHandler;
        children = new ArrayList<ElementReadHandler>();
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
        for (ElementReadHandler handler : children)
        {
            element.addNode(handler.getElement());
        }
    }

    public Element getElement()
    {
        return element;
    }
}
