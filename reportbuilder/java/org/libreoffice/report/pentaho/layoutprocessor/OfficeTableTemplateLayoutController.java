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
package org.libreoffice.report.pentaho.layoutprocessor;

import org.libreoffice.report.pentaho.OfficeNamespaces;
import org.libreoffice.report.pentaho.model.OfficeGroup;
import org.libreoffice.report.pentaho.model.OfficeReport;

import java.util.ArrayList;
import java.util.List;

import org.jfree.report.DataSourceException;
import org.jfree.report.JFreeReportInfo;
import org.jfree.report.ReportDataFactoryException;
import org.jfree.report.ReportProcessingException;
import org.jfree.report.flow.FlowController;
import org.jfree.report.flow.layoutprocessor.LayoutController;
import org.jfree.report.flow.layoutprocessor.SectionLayoutController;
import org.jfree.report.structure.Element;
import org.jfree.report.structure.Node;
import org.jfree.report.structure.Section;


/**
 * Creation-Date: 24.04.2007, 16:06:52
 *
 */
public class OfficeTableTemplateLayoutController extends SectionLayoutController
{

    private Node[] nodes;

    public OfficeTableTemplateLayoutController()
    {
    }

    /**
     * Initializes the layout controller. This method is called exactly once. It is the creators responsibility to call
     * this method.
     * <p/>
     * Calling initialize after the first advance must result in a IllegalStateException.
     *
     * @param node           the currently processed object or layout node.
     * @param flowController the current flow controller.
     * @param parent         the parent layout controller that was responsible for instantiating this controller.
     * @throws org.jfree.report.DataSourceException
     *          if there was a problem reading data from the datasource.
     * @throws org.jfree.report.ReportProcessingException
     *          if there was a general problem during the report processing.
     * @throws org.jfree.report.ReportDataFactoryException
     *          if a query failed.
     */
    public void initialize(final Object node, final FlowController flowController, final LayoutController parent)
            throws DataSourceException, ReportDataFactoryException, ReportProcessingException
    {
        final Section section = new Section();
        section.setNamespace(JFreeReportInfo.REPORT_NAMESPACE);
        section.setType("template");
        super.initialize(section, flowController, parent);

        final OfficeReport report = (OfficeReport) node;
        final ArrayList<Node> tables = new ArrayList<Node>();
        if (report.getPageHeader() != null)
        {
            addFromSection(tables, (Section) report.getPageHeader());
        }
        if (report.getReportHeader() != null)
        {
            addFromSection(tables, (Section) report.getReportHeader());
        }
        addPBody(tables, (Section) report.getPreBodySection());
        addFromBody(tables, (Section) report.getBodySection());
        addPBody(tables, (Section) report.getPostBodySection());
        if (report.getReportFooter() != null)
        {
            addFromSection(tables, (Section) report.getReportFooter());
        }
        if (report.getPageFooter() != null)
        {
            addFromSection(tables, (Section) report.getPageFooter());
        }

        this.nodes = tables.toArray(new Node[tables.size()]);
    }

    private void addPBody(final List<Node> tables, final Section section)
    {
        if (section != null)
        {
            // tables.add(section);
            final Node[] nodeArray = section.getNodeArray();
            for (int i = 0; i < nodeArray.length; i++)
            {
                final Node node = nodeArray[i];
                tables.add(node);
            }

        }
    }

    private void addFromBody(final List<Node> tables, final Section section)
    {
        final Node[] nodeArray = section.getNodeArray();
        for (int i = 0; i < nodeArray.length; i++)
        {
            final Node node = nodeArray[i];
            if (node instanceof Section)
            {
                final Section child = (Section) node;
                if (node instanceof OfficeGroup)
                {
                    addFromGroup(tables, child);
                }
                else
                {
                    addFromSection(tables, child);
                }
            }
        }
    }

    private void addFromGroup(final List<Node> tables, final Section section)
    {
        final Node[] nodeArray = section.getNodeArray();
        for (int i = 0; i < nodeArray.length; i++)
        {
            final Node node = nodeArray[i];
            if (node instanceof Section)
            {
                final Section element = (Section) node;
                if (JFreeReportInfo.REPORT_NAMESPACE.equals(element.getNamespace()) && "group-body".equals(element.getType()))
                {
                    addFromBody(tables, element);
                }
                else
                {
                    addFromSection(tables, element);
                }
            }
        }
    }

    private void addFromSection(final List<Node> tables, final Section section)
    {
        final Node[] nodeArray = section.getNodeArray();
        for (int i = 0; i < nodeArray.length; i++)
        {
            final Node node = nodeArray[i];
            if (node instanceof Element)
            {
                final Element element = (Element) node;
                if (OfficeNamespaces.TABLE_NS.equals(element.getNamespace()) && "table".equals(element.getType()))
                {
                    tables.add(element);
                }
            }
        }
    }

    public Node[] getNodes()
    {
        return nodes;
    }
}
