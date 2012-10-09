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

package com.sun.star.report.pentaho.layoutprocessor;

import com.sun.star.report.OfficeToken;
import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.pentaho.model.ImageElement;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import org.jfree.layouting.util.AttributeMap;
import org.jfree.report.DataSourceException;
import org.jfree.report.JFreeReportInfo;
import org.jfree.report.ReportDataFactoryException;
import org.jfree.report.ReportProcessingException;
import org.jfree.report.data.GlobalMasterRow;
import org.jfree.report.data.ReportDataRow;
import org.jfree.report.expressions.FormulaExpression;
import org.jfree.report.flow.FlowController;
import org.jfree.report.flow.ReportTarget;
import org.jfree.report.flow.layoutprocessor.LayoutController;
import org.jfree.report.flow.layoutprocessor.LayoutControllerUtil;
import org.jfree.report.structure.Element;
import org.jfree.report.structure.Node;
import org.jfree.report.structure.Section;
import org.jfree.report.util.TextUtilities;

import org.pentaho.reporting.libraries.base.util.ObjectUtilities;
import org.pentaho.reporting.libraries.formula.Formula;
import org.pentaho.reporting.libraries.formula.lvalues.LValue;
import org.pentaho.reporting.libraries.formula.parser.ParseException;

/**
 * Produces an image. The image-structures itself (draw:frame and so on) are not generated here. This element produces a
 * place-holder element and relies on the output target to compute a sensible position for the element. The report
 * definition does not give any hints about the size of the image, so we have to derive this from the surrounding
 * context.
 *
 * @author Thomas Morgner
 * @since 05.03.2007
 */
public class ImageElementLayoutController
        extends AbstractReportElementLayoutController
{

    private static final Log LOGGER = LogFactory.getLog(ImageElementLayoutController.class);
    private ImageElementContext context;

    public ImageElementLayoutController()
    {
    }

    protected LayoutController delegateContentGeneration(final ReportTarget target)
            throws ReportProcessingException, ReportDataFactoryException,
            DataSourceException
    {
        final ImageElement imageElement = (ImageElement) getNode();
        final FormulaExpression formulaExpression = imageElement.getFormula();
        if (formulaExpression == null)
        {
            // A static image is easy. At least at this level. Dont ask about the weird things we have to do in the
            // output targets ...
            final String linkTarget = imageElement.getImageData();
            generateImage(target, linkTarget, imageElement.getScaleMode(), imageElement.isPreserveIRI());
        }
        else
        {
            final Object value =
                    LayoutControllerUtil.evaluateExpression(getFlowController(), imageElement, formulaExpression);
            generateImage(target, value, imageElement.getScaleMode(), imageElement.isPreserveIRI());
        }
        return join(getFlowController());
    }

    private void generateImage(final ReportTarget target,
            final Object linkTarget,
            final String scale,
            final boolean preserveIri)
            throws ReportProcessingException, DataSourceException
    {
        if (linkTarget == null)
        {
            return;
        }

        final AttributeMap image = new AttributeMap();
        image.setAttribute(JFreeReportInfo.REPORT_NAMESPACE, Element.NAMESPACE_ATTRIBUTE, JFreeReportInfo.REPORT_NAMESPACE);
        image.setAttribute(JFreeReportInfo.REPORT_NAMESPACE, Element.TYPE_ATTRIBUTE, OfficeToken.IMAGE);
        image.setAttribute(JFreeReportInfo.REPORT_NAMESPACE, OfficeToken.SCALE, scale);
        image.setAttribute(JFreeReportInfo.REPORT_NAMESPACE, OfficeToken.PRESERVE_IRI, String.valueOf(preserveIri));
        image.setAttribute(JFreeReportInfo.REPORT_NAMESPACE, "image-context", createContext());
        image.setAttribute(JFreeReportInfo.REPORT_NAMESPACE, OfficeToken.IMAGE_DATA, linkTarget);
        target.startElement(image);
        target.endElement(image);
    }

    protected ImageElementContext createContext()
    {
        if (context == null)
        {

            // Step 1: Find the parent cell.
            final LayoutController cellController = findParentCell();
            if (cellController == null)
            {
                LOGGER.warn("Image is not contained in a table. Unable to calculate the image-size.");
                return null;
            }
            final Element tableCell = (Element) cellController.getNode();
            final int rowSpan = TextUtilities.parseInt((String) tableCell.getAttribute(OfficeNamespaces.TABLE_NS, "number-rows-spanned"), 1);
            final int colSpan = TextUtilities.parseInt((String) tableCell.getAttribute(OfficeNamespaces.TABLE_NS, "number-columns-spanned"), 1);
            if (rowSpan < 1 || colSpan < 1)
            {
                LOGGER.warn("Rowspan or colspan for image-size calculation was invalid.");
                return null;
            }

            final LayoutController rowController = cellController.getParent();
            if (rowController == null)
            {
                LOGGER.warn("Table-Cell has no parent. Unable to calculate the image-size.");
                return null;
            }
            final Section tableRow = (Section) rowController.getNode();
            // we are now making the assumption, that the row is a section, that contains the table-cell.
            // This breaks the ability to return nodes or to construct reports on the fly, but the OO-report format
            // is weird anyway and wont support such advanced techniques for the next few centuries ..
            final int columnPos = findNodeInSection(tableRow, tableCell, OfficeToken.COVERED_TABLE_CELL);
            if (columnPos == -1)
            {
                LOGGER.warn("Table-Cell is not a direct child of the table-row. Unable to calculate the image-size.");
                return null;
            }

            final LayoutController tableController = rowController.getParent();
            if (tableController == null)
            {
                LOGGER.warn("Table-Row has no Table. Unable to calculate the image-size.");
                return null;
            }

            final Section table = (Section) tableController.getNode();
            // ok, we got a table, so as next we have to search for the columns now.
            final Section columns = (Section) table.findFirstChild(OfficeNamespaces.TABLE_NS, OfficeToken.TABLE_COLUMNS);
            if (columns.getNodeCount() <= columnPos + colSpan)
            {
                // the colspan is to large. The table definition is therefore invalid. We do not try to fix this.
                LOGGER.warn(
                        "The Table's defined columns do not match the col-span or col-position. Unable to calculate the image-size.");
                return null;
            }

            final ImageElementContext context = new ImageElementContext(colSpan, rowSpan);
            addColumnStyles(context, columns, columnPos, colSpan);
            // finally search the styles for the row now.
            final int rowPos = findNodeInSection(table, tableRow, null);
            if (rowPos == -1)
            {
                LOGGER.warn("Table-Cell is not a direct child of the table-row. Unable to calculate the image-size.");
                return null;
            }

            addRowStyles(context, table, rowPos, rowSpan);
            this.context = context;
        }
        return this.context;
    }

    private int findNodeInSection(final Section tableRow,
            final Element tableCell,
            final String secondType)
    {
        int retval = 0;
        final Node[] nodes = tableRow.getNodeArray();
        final String namespace = tableCell.getNamespace();
        final String type = tableCell.getType();
        for (final Node node : nodes)
        {
            if (!(node instanceof Element))
            {
                continue;
            }
            final Element child = (Element) node;
            if (!ObjectUtilities.equal(child.getNamespace(), namespace) || (!ObjectUtilities.equal(child.getType(), type) && (secondType == null || !ObjectUtilities.equal(child.getType(), secondType))))
            {
                continue;
            }

            if (node == tableCell)
            {
                return retval;
            }
            retval += 1;
        }
        return -1;
    }

    private LayoutController findParentCell()
    {
        LayoutController parent = getParent();
        while (parent != null)
        {
            final Object node = parent.getNode();
            if (node instanceof Element)
            {
                final Element element = (Element) node;
                if (OfficeNamespaces.TABLE_NS.equals(element.getNamespace()) && "table-cell".equals(element.getType()))
                {
                    return parent;
                }
            }
            parent = parent.getParent();
        }
        return null;
    }

    protected boolean isValueChanged()
    {
        final ImageElement imageElement = (ImageElement) getNode();
        final FormulaExpression formulaExpression = imageElement.getFormula();
        if (formulaExpression == null)
        {
            final FlowController controller = getFlowController();
            final GlobalMasterRow masterRow = controller.getMasterRow();
            final ReportDataRow reportDataRow = masterRow.getReportDataRow();
            return reportDataRow.getCursor() == 0;
        }

        try
        {
            final Formula formula = formulaExpression.getCompiledFormula();
            final LValue lValue = formula.getRootReference();
            return isReferenceChanged(lValue);
        }
        catch (ParseException e)
        {
            return false;
        }
    }

    void addColumnStyles(final ImageElementContext context, final Section columns, final int columnPos, final int colSpan)
    {
        final Node[] columnDefs = columns.getNodeArray();
        int columnCounter = 0;
        for (Node columnDef : columnDefs)
        {
            final Element column = (Element) columnDef;

            if (!ObjectUtilities.equal(column.getNamespace(), OfficeNamespaces.TABLE_NS) || !ObjectUtilities.equal(column.getType(), OfficeToken.TABLE_COLUMN))
            {
                continue;
            }
            if (columnCounter >= columnPos)
            {
                final String colStyle = (String) column.getAttribute(OfficeNamespaces.TABLE_NS, OfficeToken.STYLE_NAME);
                context.setColStyle(columnCounter - columnPos, colStyle);
            }

            columnCounter += 1;

            if (columnCounter >= (columnPos + colSpan))
            {
                break;
            }

        }
    }

    void addRowStyles(final ImageElementContext context, final Section table, final int rowPos, final int rowSpan)
    {
        final Node[] rows = table.getNodeArray();
        int rowCounter = 0;
        for (Node row1 : rows)
        {
            final Element row = (Element) row1;

            if (!ObjectUtilities.equal(row.getNamespace(), OfficeNamespaces.TABLE_NS) || !ObjectUtilities.equal(row.getType(), OfficeToken.TABLE_ROW))
            {
                continue;
            }
            if (rowCounter >= rowPos)
            {
                final String rowStyle = (String) row.getAttribute(OfficeNamespaces.TABLE_NS, OfficeToken.STYLE_NAME);
                context.setRowStyle(rowCounter - rowPos, rowStyle);
            }

            rowCounter += 1;

            if (rowCounter >= (rowPos + rowSpan))
            {
                break;
            }
        }
    }
}
