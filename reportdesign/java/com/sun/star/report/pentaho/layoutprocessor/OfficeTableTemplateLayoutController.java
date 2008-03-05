/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OfficeTableTemplateLayoutController.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:32:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *    Copyright 2007 by Pentaho Corporation
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
package com.sun.star.report.pentaho.layoutprocessor;

import java.util.ArrayList;

import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.pentaho.model.OfficeGroup;
import com.sun.star.report.pentaho.model.OfficeReport;
import org.jfree.report.DataSourceException;
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
        section.setNamespace(OfficeNamespaces.INTERNAL_NS);
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

    private void addPBody(final ArrayList tables, final Section section)
    {
        if (section != null)
        {
            tables.add(section);
        }
//      final Node[] nodeArray = section.getNodeArray();
//    for (int i = 0; i < nodeArray.length; i++)
//    {
//      final Node node = nodeArray[i];
//      tables.add(node);
//    }
    }

    private void addFromBody(final ArrayList tables, final Section section)
    {
        final Node[] nodeArray = section.getNodeArray();
        for (int i = 0; i < nodeArray.length; i++)
        {
            final Node node = nodeArray[i];
            if (node instanceof Section == false)
            {
                continue;
            }
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

    private void addFromGroup(final ArrayList tables, final Section section)
    {
        final Node[] nodeArray = section.getNodeArray();
        for (int i = 0; i < nodeArray.length; i++)
        {
            final Node node = nodeArray[i];
            if (node instanceof Section == false)
            {
                continue;
            }

            final Section element = (Section) node;
            if (OfficeNamespaces.INTERNAL_NS.equals(element.getNamespace()) && "group-body".equals(element.getType()))
            {
                addFromBody(tables, element);
            }
            else
            {
                addFromSection(tables, element);
            }
        }
    }

    private void addFromSection(final ArrayList tables, final Section section)
    {
        final Node[] nodeArray = section.getNodeArray();
        for (int i = 0; i < nodeArray.length; i++)
        {
            final Node node = nodeArray[i];
            if (node instanceof Element == false)
            {
                continue;
            }

            final Element element = (Element) node;
            if (OfficeNamespaces.TABLE_NS.equals(element.getNamespace()) &&
                    "table".equals(element.getType()))
            {
                tables.add(element);
            }
        }
    }

    public Node[] getNodes()
    {
        return nodes;
    }
}
