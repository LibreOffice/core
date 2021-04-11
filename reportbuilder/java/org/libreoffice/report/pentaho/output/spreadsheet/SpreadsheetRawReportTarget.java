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
package org.libreoffice.report.pentaho.output.spreadsheet;

import org.libreoffice.report.DataSourceFactory;
import org.libreoffice.report.ImageService;
import org.libreoffice.report.InputRepository;
import org.libreoffice.report.OfficeToken;
import org.libreoffice.report.OutputRepository;
import org.libreoffice.report.pentaho.OfficeNamespaces;
import org.libreoffice.report.pentaho.PentahoReportEngineMetaData;
import org.libreoffice.report.pentaho.model.OfficeMasterPage;
import org.libreoffice.report.pentaho.model.OfficeMasterStyles;
import org.libreoffice.report.pentaho.model.OfficeStyle;
import org.libreoffice.report.pentaho.model.OfficeStyles;
import org.libreoffice.report.pentaho.model.OfficeStylesCollection;
import org.libreoffice.report.pentaho.model.PageSection;
import org.libreoffice.report.pentaho.output.OfficeDocumentReportTarget;
import org.libreoffice.report.pentaho.output.StyleUtilities;
import org.libreoffice.report.pentaho.output.text.MasterPageFactory;
import org.libreoffice.report.pentaho.styles.LengthCalculator;

import java.io.IOException;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.jfree.layouting.input.style.values.CSSNumericType;
import org.jfree.layouting.input.style.values.CSSNumericValue;
import org.jfree.layouting.util.AttributeMap;
import org.jfree.report.DataFlags;
import org.jfree.report.DataSourceException;
import org.jfree.report.JFreeReportInfo;
import org.jfree.report.ReportProcessingException;
import org.jfree.report.flow.ReportJob;
import org.jfree.report.flow.ReportStructureRoot;
import org.jfree.report.flow.ReportTargetUtil;
import org.jfree.report.structure.Element;
import org.jfree.report.structure.Section;
import org.jfree.report.util.IntegerCache;

import org.pentaho.reporting.libraries.resourceloader.ResourceKey;
import org.pentaho.reporting.libraries.resourceloader.ResourceManager;
import org.pentaho.reporting.libraries.xmlns.common.AttributeList;
import org.pentaho.reporting.libraries.xmlns.writer.XmlWriter;
import org.pentaho.reporting.libraries.xmlns.writer.XmlWriterSupport;


/**
 * Creation-Date: 03.11.2007
 *
 */
public class SpreadsheetRawReportTarget extends OfficeDocumentReportTarget
{

    private static final String[] FOPROPS = new String[]
    {
        "letter-spacing", "font-variant", "text-transform"
    };
    private static final String NUMBERCOLUMNSSPANNED = "number-columns-spanned";
    private static final String[] STYLEPROPS = new String[]
    {
        "text-combine", "font-pitch-complex", "text-rotation-angle", "font-name", "text-blinking", "letter-kerning", "text-combine-start-char", "text-combine-end-char", "text-position", "text-scale"
    };
    private static final int CELL_WIDTH_FACTOR = 10000;
    private static final String TRANSPARENT = "transparent";
    private boolean paragraphFound = false;
    private boolean paragraphHandled = false;

    /**
     * This class represents a column boundary, not in width, but it's actual boundary location. One of the motivations
     * for creating this class was to be able to record the boundaries for each incoming table while consuming as few
     * objects/memory as possible.
     */
    private static class ColumnBoundary implements Comparable<ColumnBoundary>
    {

        private final Set<Integer> tableIndices;
        private final long boundary;

        private ColumnBoundary(final long boundary)
        {
            this.tableIndices = new HashSet<Integer>();
            this.boundary = boundary;
        }

        public void addTableIndex(final int table)
        {
            tableIndices.add(IntegerCache.getInteger(table));
        }

        public float getBoundary()
        {
            return boundary;
        }

        public boolean isContainedByTable(final int table)
        {
            final Integer index = IntegerCache.getInteger(table);
            return tableIndices.contains(index);
        }

        public int compareTo(final ColumnBoundary arg0)
        {
            if (arg0.equals(this))
            {
                return 0;
            }
            if (boundary > arg0.boundary)
            {
                return 1;
            }
            else
            {
                return -1;
            }
        }

        @Override
        public boolean equals(final Object obj)
        {
            return obj instanceof ColumnBoundary && ((ColumnBoundary) obj).boundary == boundary;
        }

        @Override
        public int hashCode()
        {
            assert false : "hashCode not designed";
            return 42; // any arbitrary constant will do
        }
    }
    private String tableBackgroundColor; // null means transparent ...
    private boolean elementBoundaryCollectionPass;
    private boolean oleHandled;
    private final List<ColumnBoundary> columnBoundaryList;
    private long currentRowBoundaryMarker;
    private ColumnBoundary[] sortedBoundaryArray;
    private ColumnBoundary[] boundariesForTableArray;
    private int tableCounter;
    private int columnCounter;
    private int columnSpanCounter;
    private int currentSpan = 0;
    private String unitsOfMeasure;
    final private List<AttributeMap> shapes;
    final private List<AttributeMap> ole;
    final private List<CSSNumericValue> rowHeights;

    public SpreadsheetRawReportTarget(final ReportJob reportJob,
            final ResourceManager resourceManager,
            final ResourceKey baseResource,
            final InputRepository inputRepository,
            final OutputRepository outputRepository,
            final String target,
            final ImageService imageService,
            final DataSourceFactory dataSourceFactory)
            throws ReportProcessingException
    {
        super(reportJob, resourceManager, baseResource, inputRepository, outputRepository, target, imageService, dataSourceFactory);
        columnBoundaryList = new ArrayList<ColumnBoundary>();
        elementBoundaryCollectionPass = true;
        rowHeights = new ArrayList<CSSNumericValue>();
        shapes = new ArrayList<AttributeMap>();
        ole = new ArrayList<AttributeMap>();
        oleHandled = false;
    }

    @Override
    public void startOther(final AttributeMap attrs) throws DataSourceException, ReportProcessingException
    {
        if (ReportTargetUtil.isElementOfType(JFreeReportInfo.REPORT_NAMESPACE, OfficeToken.OBJECT_OLE, attrs))
        {
            if (isElementBoundaryCollectionPass() && getCurrentRole() != ROLE_TEMPLATE)
            {
                ole.add(attrs);
            }
            oleHandled = true;
            return;
        }
        final String namespace = ReportTargetUtil.getNamespaceFromAttribute(attrs);
        if (isRepeatingSection() || isFilteredNamespace(namespace))
        {
            return;
        }

        final String elementType = ReportTargetUtil.getElemenTypeFromAttribute(attrs);
        if (OfficeNamespaces.TEXT_NS.equals(namespace) && OfficeToken.P.equals(elementType) && !paragraphHandled)
        {
            paragraphFound = true;
            return;
        }

        if (OfficeNamespaces.DRAWING_NS.equals(namespace) && OfficeToken.FRAME.equals(elementType))
        {
            if (isElementBoundaryCollectionPass() && getCurrentRole() != ROLE_TEMPLATE)
            {
                final LengthCalculator len = new LengthCalculator();
                for (int i = 0; i < rowHeights.size(); i++)
                {
                    len.add(rowHeights.get(i));
                }

                rowHeights.clear();
                final CSSNumericValue currentRowHeight = len.getResult();
                rowHeights.add(currentRowHeight);
                attrs.setAttribute(OfficeNamespaces.DRAWING_NS, "z-index", String.valueOf(shapes.size()));
                final String y = (String) attrs.getAttribute(OfficeNamespaces.SVG_NS, "y");
                if (y != null)
                {
                    len.add(parseLength(y));
                    final CSSNumericValue currentY = len.getResult();
                    attrs.setAttribute(OfficeNamespaces.SVG_NS, "y", currentY.getValue() + currentY.getType().getType());
                }
                shapes.add(attrs);
            }
            return;
        }
        if (oleHandled)
        {
            if (isElementBoundaryCollectionPass() && getCurrentRole() != ROLE_TEMPLATE)
            {
                ole.add(attrs);
            }
            return;
        }

        // if this is the report namespace, write out a table definition ..
        if (OfficeNamespaces.TABLE_NS.equals(namespace) && OfficeToken.TABLE.equals(elementType))
        {
            // whenever we see a new table, we increment our tableCounter
            // this is used to keep tracked of the boundary conditions per table
            tableCounter++;
        }

        if (isElementBoundaryCollectionPass())
        {
            collectBoundaryForElement(attrs);
        }
        else
        {
            try
            {
                processElement(attrs, namespace, elementType);
            }
            catch (IOException e)
            {
                throw new ReportProcessingException(OfficeDocumentReportTarget.FAILED, e);
            }
        }
    }

    @Override
    protected void startReportSection(final AttributeMap attrs, final int role) throws ReportProcessingException
    {
        if ((role == OfficeDocumentReportTarget.ROLE_SPREADSHEET_PAGE_HEADER || role == OfficeDocumentReportTarget.ROLE_SPREADSHEET_PAGE_FOOTER) && (!PageSection.isPrintWithReportHeader(attrs) || !PageSection.isPrintWithReportFooter(attrs)))
        {
            startBuffering(new OfficeStylesCollection(), true);
        }
        else
        {
            super.startReportSection(attrs, role);
        }
    }

    @Override
    protected void endReportSection(final AttributeMap attrs, final int role) throws IOException, ReportProcessingException
    {
        if ((role == OfficeDocumentReportTarget.ROLE_SPREADSHEET_PAGE_HEADER || role == OfficeDocumentReportTarget.ROLE_SPREADSHEET_PAGE_FOOTER) && (!PageSection.isPrintWithReportHeader(attrs) || !PageSection.isPrintWithReportFooter(attrs)))
        {
            finishBuffering();
        }
        else
        {
            super.endReportSection(attrs, role);
        }
    }

    private void handleParagraph()
    {
        if (paragraphFound)
        {
            try
            {
                final XmlWriter xmlWriter = getXmlWriter();
                xmlWriter.writeTag(OfficeNamespaces.TEXT_NS, OfficeToken.P, null, XmlWriterSupport.OPEN);
                paragraphHandled = true;
                paragraphFound = false;
            }
            catch (IOException ex)
            {
                LOGGER.severe("ReportProcessing failed: " + ex);
            }
        }
    }

    private void processElement(final AttributeMap attrs, final String namespace, final String elementType)
            throws IOException, ReportProcessingException
    {
        final XmlWriter xmlWriter = getXmlWriter();

        if (ReportTargetUtil.isElementOfType(OfficeNamespaces.TABLE_NS, OfficeToken.TABLE, attrs))
        {
            // a new table means we must clear our "calculated" table boundary array cache
            boundariesForTableArray = null;

            final String tableStyle = (String) attrs.getAttribute(OfficeNamespaces.TABLE_NS, OfficeToken.STYLE_NAME);
            if (tableStyle == null)
            {
                tableBackgroundColor = null;
            }
            else
            {
                final Object raw = StyleUtilities.queryStyle(getPredefinedStylesCollection(), OfficeToken.TABLE, tableStyle,
                        "table-properties", OfficeNamespaces.FO_NS, OfficeToken.BACKGROUND_COLOR);
                if (raw == null || TRANSPARENT.equals(raw))
                {
                    tableBackgroundColor = null;
                }
                else
                {
                    tableBackgroundColor = String.valueOf(raw);
                }
            }
            return;
        }

        if (ReportTargetUtil.isElementOfType(OfficeNamespaces.TABLE_NS, OfficeToken.TABLE_COLUMN, attrs) || ReportTargetUtil.isElementOfType(OfficeNamespaces.TABLE_NS, OfficeToken.TABLE_COLUMNS, attrs))
        {
            return;
        }

        // covered-table-cell elements may appear in the input from row or column spans. In the event that we hit a
        // column-span we simply ignore these elements because we are going to adjust the span to fit the uniform table.
        if (ReportTargetUtil.isElementOfType(OfficeNamespaces.TABLE_NS, OfficeToken.COVERED_TABLE_CELL, attrs))
        {
            if (columnSpanCounter > 0)
            {
                columnSpanCounter--;
            }

            if (columnSpanCounter == 0)
            {
                // if we weren't expecting a covered-table-cell, let's use it, it's probably from a row-span
                columnCounter++;
                final int span = getColumnSpanForCell(tableCounter, columnCounter, 1);
                // use the calculated span for the column in the uniform table to create any additional covered-table-cell
                // elements
                for (int i = 0; i < span; i++)
                {
                    xmlWriter.writeTag(namespace, OfficeToken.COVERED_TABLE_CELL, null, XmlWriter.CLOSE);
                }
            }
            return;
        }

        if (ReportTargetUtil.isElementOfType(OfficeNamespaces.TABLE_NS, OfficeToken.TABLE_ROW, attrs))
        {
            // a new row means our column counter gets reset
            columnCounter = 0;
            // Lets make sure the color of the table is ok ..
            if (tableBackgroundColor != null)
            {
                final String styleName = (String) attrs.getAttribute(OfficeNamespaces.TABLE_NS, OfficeToken.STYLE_NAME);
                final OfficeStyle style = deriveStyle(OfficeToken.TABLE_ROW, styleName);
                Element tableRowProperties = style.getTableRowProperties();
                if (tableRowProperties == null)
                {
                    tableRowProperties = new Section();
                    tableRowProperties.setNamespace(OfficeNamespaces.STYLE_NS);
                    tableRowProperties.setType("table-row-properties");
                    tableRowProperties.setAttribute(OfficeNamespaces.FO_NS, OfficeToken.BACKGROUND_COLOR, tableBackgroundColor);
                    style.addNode(tableRowProperties);
                }
                else
                {
                    final Object oldValue = tableRowProperties.getAttribute(OfficeNamespaces.FO_NS, OfficeToken.BACKGROUND_COLOR);
                    if (oldValue == null || TRANSPARENT.equals(oldValue))
                    {
                        tableRowProperties.setAttribute(OfficeNamespaces.FO_NS, OfficeToken.BACKGROUND_COLOR, tableBackgroundColor);
                    }
                }
                attrs.setAttribute(OfficeNamespaces.TABLE_NS, OfficeToken.STYLE_NAME, style.getStyleName());
            }
        }
        else if (ReportTargetUtil.isElementOfType(OfficeNamespaces.TABLE_NS, OfficeToken.TABLE_CELL, attrs))
        {
            columnCounter++;
            final String styleName = (String) attrs.getAttribute(OfficeNamespaces.TABLE_NS, OfficeToken.STYLE_NAME);
            if (styleName != null)
            {
                final OfficeStyle cellStyle = getPredefinedStylesCollection().getStyle(OfficeToken.TABLE_CELL, styleName);
                if (cellStyle != null)
                {
                    final Section textProperties = (Section) cellStyle.getTextProperties();
                    if (textProperties != null)
                    {
                        for (String i : FOPROPS)
                        {
                            textProperties.setAttribute(OfficeNamespaces.FO_NS, i, null);
                        }
                        textProperties.setAttribute(OfficeNamespaces.TEXT_NS, "display", null);
                        for (String i : STYLEPROPS)
                        {
                            textProperties.setAttribute(OfficeNamespaces.STYLE_NS, i, null);
                        }
                    }
                    final Section props = (Section) cellStyle.getTableCellProperties();
                    if (props != null)
                    {
                        final Object raw = props.getAttribute(OfficeNamespaces.FO_NS, OfficeToken.BACKGROUND_COLOR);
                        if (TRANSPARENT.equals(raw))
                        {
                            props.setAttribute(OfficeNamespaces.FO_NS, OfficeToken.BACKGROUND_COLOR, null);
                        }
                    }
                }
                attrs.setAttribute(OfficeNamespaces.TABLE_NS, OfficeToken.STYLE_NAME, styleName);
            }

            final String numColSpanStr = (String) attrs.getAttribute(namespace, NUMBERCOLUMNSSPANNED);
            int initialColumnSpan = columnSpanCounter = 1;
            if (numColSpanStr != null)
            {
                initialColumnSpan = Integer.parseInt(numColSpanStr);
                columnSpanCounter = initialColumnSpan;
            }
            final int span = getColumnSpanForCell(tableCounter, columnCounter, initialColumnSpan);
            if (initialColumnSpan > 1)
            {
                // add the initial column span to our column counter index (subtract 1, since it is counted by default)
                columnCounter += initialColumnSpan - 1;
            }

            // there's no point to create number-columns-spanned attributes if we only span 1 column
            if (span > 1)
            {
                attrs.setAttribute(namespace, NUMBERCOLUMNSSPANNED, "" + span);
                currentSpan = span;
            }
            // we must also generate "covered-table-cell" elements for each column spanned
            // but we'll do this in the endElement, after we close this OfficeToken.TABLE_CELL
        }

        // All styles have to be processed or you will lose the paragraph-styles and inline text-styles.
        performStyleProcessing(attrs);

        final AttributeList attrList = buildAttributeList(attrs);
        xmlWriter.writeTag(namespace, elementType, attrList, XmlWriter.OPEN);
    }

    private void collectBoundaryForElement(final AttributeMap attrs)
    {
        if (ReportTargetUtil.isElementOfType(OfficeNamespaces.TABLE_NS, OfficeToken.TABLE_COLUMNS, attrs))
        {
            // A table row resets the column counter.
            resetCurrentRowBoundaryMarker();
        }
        else if (ReportTargetUtil.isElementOfType(OfficeNamespaces.TABLE_NS, OfficeToken.TABLE_COLUMN, attrs))
        {
            final String styleName = (String) attrs.getAttribute(OfficeNamespaces.TABLE_NS, OfficeToken.STYLE_NAME);
            if (styleName == null)
            {
                // This should not happen, but if it does, we will ignore that cell.
                return;
            }

            final OfficeStyle style = getPredefinedStylesCollection().getStyle(OfficeToken.TABLE_COLUMN, styleName);
            if (style == null)
            {
                // Now this is very bad. It means that there is no style defined with the given name.
                return;
            }

            final Element tableColumnProperties = style.getTableColumnProperties();
            String widthStr = (String) tableColumnProperties.getAttribute("column-width");
            widthStr = widthStr.substring(0, widthStr.indexOf(getUnitsOfMeasure(widthStr)));
            final float val = Float.parseFloat(widthStr) * CELL_WIDTH_FACTOR;
            addColumnWidthToRowBoundaryMarker((long) val);
            ColumnBoundary currentRowBoundary = new ColumnBoundary(getCurrentRowBoundaryMarker());
            final List<ColumnBoundary> columnBoundaryList_ = getColumnBoundaryList();
            final int idx = columnBoundaryList_.indexOf(currentRowBoundary);
            if (idx == -1)
            {
                columnBoundaryList_.add(currentRowBoundary);
            }
            else
            {
                currentRowBoundary = columnBoundaryList_.get(idx);
            }
            currentRowBoundary.addTableIndex(tableCounter);
        }
    }

    private String getUnitsOfMeasure(final String str)
    {
        if (unitsOfMeasure == null || "".equals(unitsOfMeasure))
        {
            if (str == null || "".equals(str))
            {
                unitsOfMeasure = "cm";
                return unitsOfMeasure;
            }

            // build units of measure, set it
            int i = str.length() - 1;
            for (; i >= 0; i--)
            {
                final char c = str.charAt(i);
                if (Character.isDigit(c) || c == '.' || c == ',')
                {
                    break;
                }
            }
            unitsOfMeasure = str.substring(i + 1);
        }
        return unitsOfMeasure;
    }

    private void createTableShapes() throws ReportProcessingException
    {
        if (!shapes.isEmpty())
        {
            try
            {
                final XmlWriter xmlWriter = getXmlWriter();
                // at this point we need to generate the table-columns section based on our boundary table
                // <table:shapes>
                // <draw:frame />

                // </table:shapes>
                xmlWriter.writeTag(OfficeNamespaces.TABLE_NS, OfficeToken.SHAPES, null, XmlWriterSupport.OPEN);


                for (int i = 0; i < shapes.size(); i++)
                {
                    final AttributeMap attrs = shapes.get(i);
                    final AttributeList attrList = buildAttributeList(attrs);
                    attrList.removeAttribute(OfficeNamespaces.DRAWING_NS, OfficeToken.STYLE_NAME);
                    xmlWriter.writeTag(OfficeNamespaces.DRAWING_NS, OfficeToken.FRAME, attrList, XmlWriterSupport.OPEN);
                    startChartProcessing(ole.get(i));

                    xmlWriter.writeCloseTag();
                }
                xmlWriter.writeCloseTag();
            }
            catch (IOException e)
            {
                throw new ReportProcessingException(OfficeDocumentReportTarget.FAILED, e);
            }
        }
    }

    private void createTableColumns() throws ReportProcessingException
    {
        try
        {
            final XmlWriter xmlWriter = getXmlWriter();
            // at this point we need to generate the table-columns section based on our boundary table
            // <table-columns>
            // <table-column style-name="coX"/>

            // </table-columns>
            // the first boundary is '0' which is a placeholder so we will ignore it
            xmlWriter.writeTag(OfficeNamespaces.TABLE_NS, OfficeToken.TABLE_COLUMNS, null, XmlWriterSupport.OPEN);

            // blow away current column styles
            // start processing at i=1 because we added a boundary for "0" which is virtual
            final ColumnBoundary[] cba = getSortedColumnBoundaryArray();
            for (int i = 1; i < cba.length; i++)
            {
                final ColumnBoundary cb = cba[i];
                float columnWidth = cb.getBoundary();
                if (i > 1)
                {
                    columnWidth -= cba[i - 1].getBoundary();
                }
                columnWidth = columnWidth / CELL_WIDTH_FACTOR;
                final OfficeStyle style = deriveStyle(OfficeToken.TABLE_COLUMN, ("co" + i + "_"));
                final Section tableColumnProperties = new Section();
                tableColumnProperties.setType("table-column-properties");
                tableColumnProperties.setNamespace(style.getNamespace());
                final String width = String.format("%f", columnWidth);
                tableColumnProperties.setAttribute(style.getNamespace(),
                        "column-width", width + getUnitsOfMeasure(null));
                style.addNode(tableColumnProperties);

                final AttributeList myAttrList = new AttributeList();
                myAttrList.setAttribute(OfficeNamespaces.TABLE_NS, OfficeToken.STYLE_NAME, style.getStyleName());
                xmlWriter.writeTag(OfficeNamespaces.TABLE_NS, OfficeToken.TABLE_COLUMN, myAttrList, XmlWriterSupport.CLOSE);
            }
            xmlWriter.writeCloseTag();
        }
        catch (IOException e)
        {
            throw new ReportProcessingException(OfficeDocumentReportTarget.FAILED, e);
        }
    }

    @Override
    protected void endOther(final AttributeMap attrs) throws DataSourceException, ReportProcessingException
    {
        if (ReportTargetUtil.isElementOfType(JFreeReportInfo.REPORT_NAMESPACE, OfficeToken.OBJECT_OLE, attrs) || oleHandled)
        {
            oleHandled = false;
            return;
        }

        if (ReportTargetUtil.isElementOfType(OfficeNamespaces.TABLE_NS, OfficeToken.TABLE_ROW, attrs) && isElementBoundaryCollectionPass() && getCurrentRole() != ROLE_TEMPLATE)
        {
            final String styleName = (String) attrs.getAttribute(OfficeNamespaces.TABLE_NS, OfficeToken.STYLE_NAME);
            rowHeights.add(computeRowHeight(styleName));
        }

        if (isRepeatingSection() || isElementBoundaryCollectionPass())
        {
            return;
        }

        final String namespace = ReportTargetUtil.getNamespaceFromAttribute(attrs);
        if (isFilteredNamespace(namespace))
        {
            return;
        }
        final String elementType = ReportTargetUtil.getElemenTypeFromAttribute(attrs);
        if (OfficeNamespaces.DRAWING_NS.equals(namespace) && OfficeToken.FRAME.equals(elementType))
        {
            return;
        }

        // if this is the report namespace, write out a table definition ..
        if (OfficeNamespaces.TABLE_NS.equals(namespace) && (OfficeToken.TABLE.equals(elementType) || OfficeToken.COVERED_TABLE_CELL.equals(elementType) || OfficeToken.TABLE_COLUMN.equals(elementType) || OfficeToken.TABLE_COLUMNS.equals(elementType)))
        {
            return;
        }

        if (!paragraphHandled && OfficeNamespaces.TEXT_NS.equals(namespace) && OfficeToken.P.equals(elementType))
        {
            if (!paragraphHandled)
            {
                return;
            }

            paragraphHandled = false;
        }
        try
        {
            final XmlWriter xmlWriter = getXmlWriter();
            xmlWriter.writeCloseTag();
            // table-cell elements may have a number-columns-spanned attribute which indicates how many
            // 'covered-table-cell' elements we need to generate
            generateCoveredTableCells(attrs);
        }
        catch (IOException e)
        {
            throw new ReportProcessingException(OfficeDocumentReportTarget.FAILED, e);
        }
    }

    private void generateCoveredTableCells(final AttributeMap attrs) throws IOException
    {
        if (!ReportTargetUtil.isElementOfType(OfficeNamespaces.TABLE_NS, OfficeToken.TABLE_CELL, attrs))
        {
            return;
        }

        // do this after we close the tag
        final XmlWriter xmlWriter = getXmlWriter();
        final int span = currentSpan;
        currentSpan = 0;
        for (int i = 1; i < span; i++)
        {
            xmlWriter.writeTag(OfficeNamespaces.TABLE_NS, OfficeToken.COVERED_TABLE_CELL, null, XmlWriter.CLOSE);
        }
    }

    public String getExportDescriptor()
    {
        return "raw/" + PentahoReportEngineMetaData.OPENDOCUMENT_SPREADSHEET;
    }


    @Override
    public void processText(final String text) throws DataSourceException, ReportProcessingException
    {
        if (!(isRepeatingSection() || isElementBoundaryCollectionPass()))
        {
            handleParagraph();
            super.processText(text);
        }
    }

    @Override
    public void processContent(final DataFlags value) throws DataSourceException, ReportProcessingException
    {
        if (!(isRepeatingSection() || isElementBoundaryCollectionPass()))
        {
            handleParagraph();
            super.processContent(value);
        }
    }

    private String getStartContent()
    {
        return "spreadsheet";
    }

    @Override
    protected void startContent(final AttributeMap attrs) throws IOException, DataSourceException,
            ReportProcessingException
    {
        if (!isElementBoundaryCollectionPass())
        {
            final XmlWriter xmlWriter = getXmlWriter();
            xmlWriter.writeTag(OfficeNamespaces.OFFICE_NS, getStartContent(), null, XmlWriterSupport.OPEN);

            writeNullDate();

            final AttributeMap tableAttributes = new AttributeMap();
            tableAttributes.setAttribute(JFreeReportInfo.REPORT_NAMESPACE, Element.NAMESPACE_ATTRIBUTE, OfficeNamespaces.TABLE_NS);
            tableAttributes.setAttribute(JFreeReportInfo.REPORT_NAMESPACE, Element.TYPE_ATTRIBUTE, OfficeToken.TABLE);
            tableAttributes.setAttribute(OfficeNamespaces.TABLE_NS, OfficeToken.STYLE_NAME, generateInitialTableStyle());
            tableAttributes.setAttribute(OfficeNamespaces.TABLE_NS, "name", "Report");

            performStyleProcessing(tableAttributes);

            xmlWriter.writeTag(OfficeNamespaces.TABLE_NS, OfficeToken.TABLE, buildAttributeList(tableAttributes), XmlWriterSupport.OPEN);
            createTableShapes();
            createTableColumns();
        }
    }

    private String generateInitialTableStyle() throws ReportProcessingException
    {
        final OfficeStylesCollection predefStyles = getPredefinedStylesCollection();
        final OfficeStyles commonStyles = predefStyles.getAutomaticStyles();
        if (!commonStyles.containsStyle(OfficeToken.TABLE, "Initial_Table"))
        {
            final String masterPageName = createMasterPage();

            final OfficeStyle tableStyle = new OfficeStyle();
            tableStyle.setStyleFamily(OfficeToken.TABLE);
            tableStyle.setStyleName("Initial_Table");
            tableStyle.setAttribute(OfficeNamespaces.STYLE_NS, "master-page-name", masterPageName);
            final Element tableProperties = produceFirstChild(tableStyle, OfficeNamespaces.STYLE_NS, "table-properties");
            tableProperties.setAttribute(OfficeNamespaces.FO_NS, OfficeToken.BACKGROUND_COLOR, TRANSPARENT);
            commonStyles.addStyle(tableStyle);
        }
        return "Initial_Table";
    }

    private String createMasterPage() throws ReportProcessingException
    {
        final OfficeStylesCollection predefStyles = getPredefinedStylesCollection();
        final MasterPageFactory masterPageFactory = new MasterPageFactory(predefStyles.getMasterStyles());
        final OfficeMasterPage masterPage;
        if (!masterPageFactory.containsMasterPage("Standard", null, null))
        {
            masterPage = masterPageFactory.createMasterPage("Standard", null, null);

            final CSSNumericValue zeroLength = CSSNumericValue.createValue(CSSNumericType.CM, 0);
            final String pageLayoutTemplate = masterPage.getPageLayout();
            if (pageLayoutTemplate == null)
            {
                // there is no pagelayout. Create one ..
                final String derivedLayout = masterPageFactory.createPageStyle(getGlobalStylesCollection().getAutomaticStyles(), zeroLength, zeroLength);
                masterPage.setPageLayout(derivedLayout);
            }
            else
            {
                final String derivedLayout = masterPageFactory.derivePageStyle(pageLayoutTemplate,
                        getPredefinedStylesCollection().getAutomaticStyles(),
                        getGlobalStylesCollection().getAutomaticStyles(), zeroLength, zeroLength);
                masterPage.setPageLayout(derivedLayout);
            }

            final OfficeStylesCollection officeStylesCollection = getGlobalStylesCollection();
            final OfficeMasterStyles officeMasterStyles = officeStylesCollection.getMasterStyles();
            officeMasterStyles.addMasterPage(masterPage);
        }
        else
        {
            masterPage = masterPageFactory.getMasterPage("Standard", null, null);
        }
        return masterPage.getStyleName();
    }

    @Override
    protected void endContent(final AttributeMap attrs) throws IOException, DataSourceException,
            ReportProcessingException
    {
        // todo
        if (!isElementBoundaryCollectionPass())
        {
            final XmlWriter xmlWriter = getXmlWriter();
            xmlWriter.writeCloseTag();
            xmlWriter.writeCloseTag();
        }
    }

    @Override
    public void endReport(final ReportStructureRoot report) throws DataSourceException, ReportProcessingException
    {
        super.endReport(report);
        setElementBoundaryCollectionPass(false);
        resetTableCounter();
        columnCounter = 0;
        copyMeta();
    }

    private boolean isElementBoundaryCollectionPass()
    {
        return elementBoundaryCollectionPass;
    }

    private void setElementBoundaryCollectionPass(final boolean elementBoundaryCollectionPass)
    {
        this.elementBoundaryCollectionPass = elementBoundaryCollectionPass;
    }

    private ColumnBoundary[] getSortedColumnBoundaryArray()
    {
        if (sortedBoundaryArray == null)
        {
            getColumnBoundaryList().add(new ColumnBoundary(0));
            sortedBoundaryArray = getColumnBoundaryList().toArray(new ColumnBoundary[getColumnBoundaryList().size()]);
            Arrays.sort(sortedBoundaryArray);
        }
        return sortedBoundaryArray;
    }

    private List<ColumnBoundary> getColumnBoundaryList()
    {
        return columnBoundaryList;
    }

    private void addColumnWidthToRowBoundaryMarker(final long width)
    {
        currentRowBoundaryMarker += width;
    }

    private long getCurrentRowBoundaryMarker()
    {
        return currentRowBoundaryMarker;
    }

    private void resetTableCounter()
    {
        tableCounter = 0;
    }

    private void resetCurrentRowBoundaryMarker()
    {
        currentRowBoundaryMarker = 0;
    }

    private ColumnBoundary[] getBoundariesForTable(final int table)
    {
        if (boundariesForTableArray == null)
        {
            final List<ColumnBoundary> boundariesForTable = new ArrayList<ColumnBoundary>();
            final List<ColumnBoundary> boundaryList = getColumnBoundaryList();
            for (int i = 0; i < boundaryList.size(); i++)
            {
                final ColumnBoundary b = boundaryList.get(i);
                if (b.isContainedByTable(table))
                {
                    boundariesForTable.add(b);
                }
            }
            boundariesForTableArray = boundariesForTable.toArray(new ColumnBoundary[boundariesForTable.size()]);
            Arrays.sort(boundariesForTableArray);
        }
        return boundariesForTableArray;
    }

    private int getColumnSpanForCell(final int table, final int col, final int initialColumnSpan)
    {
        final ColumnBoundary[] globalBoundaries = getSortedColumnBoundaryArray();
        final ColumnBoundary[] tableBoundaries = getBoundariesForTable(table);
        // how many column boundaries in the globalBoundaries list fall between the currentRowWidth and the next boundary
        // for the current row

        float cellBoundary = tableBoundaries[col - 1].getBoundary();
        float cellWidth = tableBoundaries[col - 1].getBoundary();

        if (col > 1)
        {
            cellWidth = cellWidth - tableBoundaries[col - 2].getBoundary();
        }

        if (initialColumnSpan > 1)
        {
            // ok we've got some additional spanning specified on the input
            final int index = (col - 1) + (initialColumnSpan - 1);
            cellWidth += tableBoundaries[index].getBoundary() - tableBoundaries[col - 1].getBoundary();
            cellBoundary = tableBoundaries[index].getBoundary();
        }

        int beginBoundaryIndex = 0;
        int endBoundaryIndex = globalBoundaries.length - 1;
        for (int i = 0; i < globalBoundaries.length; i++)
        {
            // find beginning boundary
            if (globalBoundaries[i].getBoundary() <= cellBoundary - cellWidth)
            {
                beginBoundaryIndex = i;
            }
            if (globalBoundaries[i].getBoundary() <= cellBoundary)
            {
                endBoundaryIndex = i;
            }
        }
        final int span = endBoundaryIndex - beginBoundaryIndex;
        // span will be zero for the first column, so we adjust it to 1
        if (span == 0)
        {
            return 1;
        }
        return span;
    }

    @Override
    protected String getTargetMimeType()
    {
        return "application/vnd.oasis.opendocument.spreadsheet";
    }
}
