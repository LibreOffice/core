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


package com.sun.star.report.pentaho.parser.chart;

import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.pentaho.parser.ElementReadHandler;
import com.sun.star.report.pentaho.parser.rpt.DetailRootTableReadHandler;
import com.sun.star.report.pentaho.parser.rpt.ReportReadHandler;
import com.sun.star.report.pentaho.parser.text.TextContentReadHandler;

import java.lang.Object;

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
        for (Object aChildren : children)
        {
            final ElementReadHandler handler = (ElementReadHandler) aChildren;
            element.addNode(handler.getElement());
        }
    }

    public Element getElement()
    {
        return element;
    }
}
