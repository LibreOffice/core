/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ReportReadHandler.java,v $
 * $Revision: 1.6 $
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
package com.sun.star.report.pentaho.parser.rpt;

import java.util.ArrayList;

import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.pentaho.model.OfficeReport;
import com.sun.star.report.pentaho.parser.ElementReadHandler;
import com.sun.star.report.pentaho.parser.chart.ChartReadHandler;
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

    public void setDetail(final RootTableReadHandler detail)
    {
        this.detail = detail;
    }
    public final RootTableReadHandler getDetail()
    {
        return detail;
    }
    private GroupReadHandler groups;
    private final OfficeReport rootSection;
    private final List functionHandlers;
    private final List preBodyHandlers;
    private final List postBodyHandlers;
    private boolean pre = true;

    public ReportReadHandler()
    {
        rootSection = new OfficeReport();
        rootSection.setAttribute(JFreeReportInfo.REPORT_NAMESPACE, "simple-report-structure", Boolean.TRUE);
        functionHandlers = new ArrayList();
        preBodyHandlers = new ArrayList();
        postBodyHandlers = new ArrayList();
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
            erh = new ChartReadHandler(this);
            if (pre)
            {
                preBodyHandlers.add(erh);
            }
            else
            {
                postBodyHandlers.add(erh);
            }
        }
        else if (OfficeNamespaces.OOREPORT_NS.equals(uri))
        {
            if ("function".equals(tagName))
            {
                erh = new FunctionReadHandler();
                functionHandlers.add(erh);
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
            final FunctionReadHandler handler =
                    (FunctionReadHandler) functionHandlers.get(i);
            rootSection.addExpression(handler.getExpression());
        }
    }

    public Element getElement()
    {
        return rootSection;
    }

    private final Section createSection(final String name, final List handler)
    {
        if (!handler.isEmpty())
        {
            final Section section = new Section();
            section.setNamespace(JFreeReportInfo.REPORT_NAMESPACE);
            section.setType(name);

            for (int i = 0; i < handler.size(); i++)
            {
                final ElementReadHandler erh = (ElementReadHandler) handler.get(i);
                section.addNode(erh.getElement());
            }
            return section;
        }
        return null;
    }
}
