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

import org.libreoffice.report.OfficeToken;
import org.libreoffice.report.pentaho.OfficeNamespaces;
import org.libreoffice.report.pentaho.model.OfficeGroup;
import org.libreoffice.report.pentaho.model.OfficeGroupInstanceSection;
import org.libreoffice.report.pentaho.parser.ElementReadHandler;

import java.util.ArrayList;
import java.util.List;

import org.jfree.report.JFreeReportInfo;
import org.jfree.report.expressions.FormulaExpression;
import org.jfree.report.structure.Element;
import org.jfree.report.structure.Section;

import org.pentaho.reporting.libraries.xmlns.parser.XmlReadHandler;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;


public class GroupReadHandler extends ElementReadHandler
{

    private GroupSectionReadHandler groupHeader;
    private GroupSectionReadHandler groupFooter;
    private GroupReadHandler childGroup;
    private RootTableReadHandler detailSection;
    private final OfficeGroup group;
    private final OfficeGroupInstanceSection groupInstanceSection;
    private final List<FunctionReadHandler> functionHandlers;
    private final ReportReadHandler rh;

    public GroupReadHandler(final ReportReadHandler _rh)
    {
        rh = _rh;
        group = new OfficeGroup();
        groupInstanceSection = new OfficeGroupInstanceSection();
        groupInstanceSection.setNamespace(JFreeReportInfo.REPORT_NAMESPACE);
        groupInstanceSection.setType("group-instance");
        group.addNode(groupInstanceSection);
        functionHandlers = new ArrayList<FunctionReadHandler>();
    }

    /**
     * Starts parsing.
     *
     * @param attrs the attributes.
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
    protected void startParsing(final Attributes attrs) throws SAXException
    {
        super.startParsing(attrs);

        final String groupExpr = attrs.getValue(OfficeNamespaces.OOREPORT_NS, "group-expression");
        if (groupExpr != null && !"".equals(groupExpr))
        {
            final FormulaExpression function = new FormulaExpression();
            function.setFormula(groupExpr);
            groupInstanceSection.setGroupingExpression(function);
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
        if (!OfficeNamespaces.OOREPORT_NS.equals(uri))
        {
            return null;
        }
        if ("function".equals(tagName))
        {
            final FunctionReadHandler erh = new FunctionReadHandler();
            functionHandlers.add(erh);
            return erh;
        }
        if ("group-header".equals(tagName))
        {
            groupHeader = new GroupSectionReadHandler();
            return groupHeader;
        }
        if ("group".equals(tagName))
        {
            childGroup = new GroupReadHandler(rh);
            return childGroup;
        }
        if ("detail".equals(tagName))
        {
            detailSection = new DetailRootTableReadHandler();
            rh.setDetail(detailSection);
            return detailSection;
        }
        if ("group-footer".equals(tagName))
        {
            ((Element) ((Section) rh.getDetail().getElement()).getNode(0)).setAttribute(JFreeReportInfo.REPORT_NAMESPACE, "has-group-footer", OfficeToken.TRUE);
            groupFooter = new GroupSectionReadHandler();
            return groupFooter;
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
        for (int i = 0; i < functionHandlers.size(); i++)
        {
            final FunctionReadHandler handler = functionHandlers.get(i);
            groupInstanceSection.addExpression(handler.getExpression());
        }

        if (groupHeader != null)
        {
            groupInstanceSection.addNode(groupHeader.getElement());
        }

        final Section groupBody = new Section();
        groupBody.setNamespace(JFreeReportInfo.REPORT_NAMESPACE);
        groupBody.setType("group-body");
        groupInstanceSection.addNode(groupBody);
        // XOR: Either the detail or the group section can be set ..
        if (detailSection != null)
        {
            groupBody.addNode(detailSection.getElement());
        }
        else if (childGroup != null)
        {
            groupBody.addNode(childGroup.getElement());
        }

        if (groupFooter != null)
        {
            groupInstanceSection.addNode(groupFooter.getElement());
        }
    }

    public Element getElement()
    {
        return group;
    }
}
