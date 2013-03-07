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
package org.libreoffice.report.pentaho.parser.rpt;

import org.libreoffice.report.pentaho.OfficeNamespaces;
import org.libreoffice.report.pentaho.model.OfficeReport;
import org.libreoffice.report.pentaho.parser.ElementReadHandler;
import org.libreoffice.report.pentaho.parser.chart.ChartReadHandler;

import java.util.ArrayList;
import java.util.List;

import org.jfree.report.JFreeReportInfo;
import org.jfree.report.structure.Element;
import org.jfree.report.structure.Section;

import org.pentaho.reporting.libraries.xmlns.parser.XmlReadHandler;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;


public class ReportReadHandler extends ElementReadHandler
{

    private RootTableReadHandler pageHeader;
    private RootTableReadHandler pageFooter;
    private RootTableReadHandler reportHeader;
    private RootTableReadHandler reportFooter;
    private RootTableReadHandler detail;
    private GroupReadHandler groups;
    private final OfficeReport rootSection;
    private final List<FunctionReadHandler> functionHandlers;
    private final List<ElementReadHandler> preBodyHandlers;
    private final List<ElementReadHandler> postBodyHandlers;
    private boolean pre = true;

    public ReportReadHandler()
    {
        rootSection = new OfficeReport();
        rootSection.setAttribute(JFreeReportInfo.REPORT_NAMESPACE, "simple-report-structure", Boolean.TRUE);
        functionHandlers = new ArrayList<FunctionReadHandler>();
        preBodyHandlers = new ArrayList<ElementReadHandler>();
        postBodyHandlers = new ArrayList<ElementReadHandler>();
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
        final XmlReadHandler erh;
        if (OfficeNamespaces.CHART_NS.equals(uri))
        {
            ChartReadHandler crh = new ChartReadHandler(this);
            if (pre)
            {
                preBodyHandlers.add(crh);
            }
            else
            {
                postBodyHandlers.add(crh);
            }
            erh = crh;
        }
        else if (OfficeNamespaces.OOREPORT_NS.equals(uri))
        {
            if ("function".equals(tagName))
            {
                FunctionReadHandler frh = new FunctionReadHandler();
                functionHandlers.add(frh);
                erh = frh;
            }
            else if ("page-header".equals(tagName))
            {
                pageHeader = new RootTableReadHandler();
                erh = pageHeader;
            }
            else if ("report-header".equals(tagName))
            {
                reportHeader = new RootTableReadHandler();
                erh = reportHeader;
            }
            else if ("report-footer".equals(tagName))
            {
                reportFooter = new RootTableReadHandler();
                erh = reportFooter;
            }
            else if ("page-footer".equals(tagName))
            {
                pageFooter = new RootTableReadHandler();
                erh = pageFooter;
            }
            else if ("detail".equals(tagName))
            {
                pre = false;
                detail = new DetailRootTableReadHandler();
                erh = detail;
            }
            else if ("group".equals(tagName))
            {
                groups = new GroupReadHandler(this);
                erh = groups;
            }
            else
            {
                erh = null;
            }
        }
        else
        {
            erh = null;
        }
        return erh;
    }

    /**
     * Done parsing.
     *
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
    protected void doneParsing() throws SAXException
    {
        if (pageHeader != null)
        {
            rootSection.setPageHeader(pageHeader.getElement());
        }
        if (pageFooter != null)
        {
            rootSection.setPageFooter(pageFooter.getElement());
        }
        if (reportHeader != null)
        {
            rootSection.setReportHeader(reportHeader.getElement());
        }

        final Section preBody = createSection("report-pre-body", preBodyHandlers);
        if (preBody != null)
        {
            rootSection.setPreBodySection(preBody);
        }

        final Section groupBody = new Section();
        groupBody.setNamespace(JFreeReportInfo.REPORT_NAMESPACE);
        groupBody.setType("report-body");
        rootSection.setBodySection(groupBody);

        // XOR: Either the detail or the group section can be set ..
        if (groups != null)
        {
            groupBody.addNode(groups.getElement());
        }
        else if (detail != null)
        {
            groupBody.addNode(detail.getElement());
        }

        final Section postBody = createSection("report-post-body", postBodyHandlers);
        if (postBody != null)
        {
            rootSection.setPostBodySection(postBody);
        }

        if (reportFooter != null)
        {
            rootSection.setReportFooter(reportFooter.getElement());
        }

        for (int i = 0; i < functionHandlers.size(); i++)
        {
            final FunctionReadHandler handler = functionHandlers.get(i);
            rootSection.addExpression(handler.getExpression());
        }
    }

    public Element getElement()
    {
        return rootSection;
    }

    private final Section createSection(final String name, final List<ElementReadHandler> handler)
    {
        if (!handler.isEmpty())
        {
            final Section section = new Section();
            section.setNamespace(JFreeReportInfo.REPORT_NAMESPACE);
            section.setType(name);

            for (int i = 0; i < handler.size(); i++)
            {
                final ElementReadHandler erh = handler.get(i);
                section.addNode(erh.getElement());
            }
            return section;
        }
        return null;
    }

    public void setDetail(final RootTableReadHandler detail)
    {
        this.detail = detail;
    }

    public final RootTableReadHandler getDetail()
    {
        return detail;
    }

}
