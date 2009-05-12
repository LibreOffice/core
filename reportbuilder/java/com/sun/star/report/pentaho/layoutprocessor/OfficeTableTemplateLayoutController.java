/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OfficeTableTemplateLayoutController.java,v $
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
package com.sun.star.report.pentaho.layoutprocessor;

import java.util.ArrayList;

import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.pentaho.model.OfficeGroup;
import com.sun.star.report.pentaho.model.OfficeReport;
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
 * @author Thomas Morgner
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
        final ArrayList tables = new ArrayList();
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

        this.nodes = (Node[]) tables.toArray(new Node[tables.size()]);
    }

    private void addPBody(final List tables, final Section section)
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

    private void addFromBody(final List tables, final Section section)
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

    private void addFromGroup(final List tables, final Section section)
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

    private void addFromSection(final List tables, final Section section)
    {
        final Node[] nodeArray = section.getNodeArray();
        for (int i = 0; i < nodeArray.length; i++)
        {
            final Node node = nodeArray[i];
            if (node instanceof Element)
            {
                final Element element = (Element) node;
                if (OfficeNamespaces.TABLE_NS.equals(element.getNamespace()) &&
                        "table".equals(element.getType()))
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
