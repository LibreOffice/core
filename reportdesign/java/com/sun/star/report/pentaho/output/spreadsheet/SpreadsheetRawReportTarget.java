/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SpreadsheetRawReportTarget.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 14:34:57 $
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

package com.sun.star.report.pentaho.output.spreadsheet;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;

import com.sun.star.report.InputRepository;
import com.sun.star.report.OutputRepository;
import com.sun.star.report.ImageService;
import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.pentaho.PentahoReportEngineMetaData;
import com.sun.star.report.pentaho.model.OfficeStyle;
import com.sun.star.report.pentaho.model.OfficeStyles;
import com.sun.star.report.pentaho.model.OfficeStylesCollection;
import com.sun.star.report.pentaho.model.OfficeMasterPage;
import com.sun.star.report.pentaho.model.OfficeMasterStyles;
import com.sun.star.report.pentaho.model.PageSection;
import com.sun.star.report.pentaho.output.OfficeDocumentReportTarget;
import com.sun.star.report.pentaho.output.StyleUtilities;
import com.sun.star.report.pentaho.output.text.MasterPageFactory;
import org.jfree.layouting.util.AttributeMap;
import org.jfree.layouting.input.style.values.CSSNumericValue;
import org.jfree.layouting.input.style.values.CSSNumericType;
import org.jfree.report.DataFlags;
import org.jfree.report.DataSourceException;
import org.jfree.report.ReportProcessingException;
import org.jfree.report.JFreeReportInfo;
import org.jfree.report.flow.ReportJob;
import org.jfree.report.flow.ReportStructureRoot;
import org.jfree.report.flow.ReportTargetUtil;
import org.jfree.report.structure.Element;
import org.jfree.report.structure.Section;
import org.jfree.report.util.IntegerCache;
import org.jfree.report.util.TextUtilities;
import org.jfree.resourceloader.ResourceKey;
import org.jfree.resourceloader.ResourceManager;
import org.jfree.xmlns.common.AttributeList;
import org.jfree.xmlns.writer.XmlWriter;
import org.jfree.xmlns.writer.XmlWriterSupport;

/**
 * Creation-Date: 03.11.2007
 *
 * @author Michael D'Amour
 */
public class SpreadsheetRawReportTarget extends OfficeDocumentReportTarget
{

  /**
   * This class represents a column boundary, not in width, but it's actual boundary location. One of the motivations
   * for creating this class was to be able to record the boundaries for each incoming table while consuming as few
   * objects/memory as possible.
   */
  private static class ColumnBoundary implements Comparable
  {
    private HashSet tableIndices;

    private float boundary;

    private ColumnBoundary(final float boundary)
    {
      this.tableIndices = new HashSet();
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

    public int compareTo(final Object arg0)
    {
      if (arg0.equals(this))
      {
        return 0;
      }
      if (arg0 instanceof ColumnBoundary)
      {
        if (boundary > ((ColumnBoundary) arg0).boundary)
        {
          return 1;
        }
        else
        {
          return -1;
        }
      }
      return 1;
    }

    public boolean equals(final Object obj)
    {
      if (obj instanceof ColumnBoundary)
      {
        return ((ColumnBoundary) obj).boundary == boundary;
      }
      return false;
    }
  }

  private String tableBackgroundColor; // null means transparent ...

  private static final ColumnBoundary[] EMPTY_COLBOUNDS = new ColumnBoundary[0];

  private boolean elementBoundaryCollectionPass;
  private List columnBoundaryList;
  private float currentRowBoundaryMarker;

  private ColumnBoundary[] sortedBoundaryArray;
  private ColumnBoundary[] boundariesForTableArray;

  private int tableCounter;
  private int columnCounter;
  private int columnSpanCounter;
  private String unitsOfMeasure;

  public SpreadsheetRawReportTarget(final ReportJob reportJob,
                                    final ResourceManager resourceManager,
                                    final ResourceKey baseResource,
                                    final InputRepository inputRepository,
                                    final OutputRepository outputRepository,
                                    final String target,
                                    final ImageService imageService)
      throws ReportProcessingException
  {
    super(reportJob, resourceManager, baseResource, inputRepository, outputRepository, target, imageService);
    columnBoundaryList = new ArrayList();
    elementBoundaryCollectionPass = true;
  }

  public void startOther(final AttributeMap attrs) throws DataSourceException, ReportProcessingException
  {
    final String namespace = ReportTargetUtil.getNamespaceFromAttribute(attrs);
    if (isRepeatingSection() || isFilteredNamespace(namespace))
    {
      return;
    }

    final String elementType = ReportTargetUtil.getElemenTypeFromAttribute(attrs);
    // if this is the report namespace, write out a table definition ..
    if (OfficeNamespaces.TABLE_NS.equals(namespace) && "table".equals(elementType))
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
    // if (!isElementBoundaryCollectionPass())
    {
      try
      {
        processElement(attrs, namespace, elementType);
      }
      catch (IOException e)
      {
        throw new ReportProcessingException("Failed", e);
      }
    }
  }

  protected void startReportSection(final AttributeMap attrs, final int role) throws IOException, DataSourceException, ReportProcessingException
  {
    if ((role == OfficeDocumentReportTarget.ROLE_SPREADSHEET_PAGE_HEADER ||
         role == OfficeDocumentReportTarget.ROLE_SPREADSHEET_PAGE_FOOTER) &&
        (PageSection.isPrintWithReportHeader(attrs) == false ||
         PageSection.isPrintWithReportFooter(attrs) == false))
    {
      startBuffering(new OfficeStylesCollection(), true);
      return;
    }
    super.startReportSection(attrs, role);
  }

  protected void endReportSection(final AttributeMap attrs, final int role) throws IOException, DataSourceException, ReportProcessingException
  {
    if ((role == OfficeDocumentReportTarget.ROLE_SPREADSHEET_PAGE_HEADER ||
         role == OfficeDocumentReportTarget.ROLE_SPREADSHEET_PAGE_FOOTER) &&
        (PageSection.isPrintWithReportHeader(attrs) == false ||
         PageSection.isPrintWithReportFooter(attrs) == false))
    {
      finishBuffering();
      return;
    }

    super.endReportSection(attrs, role);
  }

  private void processElement(final AttributeMap attrs, final String namespace, final String elementType)
      throws IOException, ReportProcessingException
  {
    final XmlWriter xmlWriter = getXmlWriter();

    if (ReportTargetUtil.isElementOfType(OfficeNamespaces.TABLE_NS, "table", attrs))
    {
      // a new table means we must clear our "calculated" table boundary array cache
      boundariesForTableArray = null;

      final String tableStyle = (String) attrs.getAttribute(OfficeNamespaces.TABLE_NS, "style-name");
      if (tableStyle == null)
      {
        tableBackgroundColor = null;
      }
      else
      {
        final Object raw = StyleUtilities.queryStyle(getPredefinedStylesCollection(), "table", tableStyle,
            "table-properties", OfficeNamespaces.FO_NS, "background-color");
        if (raw == null || "transparent".equals(raw))
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

    if (ReportTargetUtil.isElementOfType(OfficeNamespaces.TABLE_NS, "table-column", attrs)
        || ReportTargetUtil.isElementOfType(OfficeNamespaces.TABLE_NS, "table-columns", attrs))
    {
      return;
    }

    // covered-table-cell elements may appear in the input from row or column spans. In the event that we hit a
    // column-span we simply ignore these elements because we are going to adjust the span to fit the uniform table.
    if (ReportTargetUtil.isElementOfType(OfficeNamespaces.TABLE_NS, "covered-table-cell", attrs))
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
          xmlWriter.writeTag(namespace, "covered-table-cell", null, XmlWriter.CLOSE);
        }
      }
      return;
    }

    if (ReportTargetUtil.isElementOfType(OfficeNamespaces.TABLE_NS, "table-row", attrs))
    {
      // a new row means our column counter gets reset
      columnCounter = 0;
      // Lets make sure the color of the table is ok ..
      if (tableBackgroundColor != null)
      {
        final String styleName = (String) attrs.getAttribute(OfficeNamespaces.TABLE_NS, "style-name");
        final OfficeStyle style = deriveStyle("table-row", styleName);
        Element tableRowProperties = style.getTableRowProperties();
        if (tableRowProperties == null)
        {
          tableRowProperties = new Section();
          tableRowProperties.setNamespace(OfficeNamespaces.STYLE_NS);
          tableRowProperties.setType("table-row-properties");
          tableRowProperties.setAttribute(OfficeNamespaces.FO_NS, "background-color", tableBackgroundColor);
          style.addNode(tableRowProperties);
        }
        else
        {
          final Object oldValue = tableRowProperties.getAttribute(OfficeNamespaces.FO_NS, "background-color");
          if (oldValue == null || "transparent".equals(oldValue))
          {
            tableRowProperties.setAttribute(OfficeNamespaces.FO_NS, "background-color", tableBackgroundColor);
          }
        }
        attrs.setAttribute(OfficeNamespaces.TABLE_NS, "style-name", style.getStyleName());
      }
    }
    else if (ReportTargetUtil.isElementOfType(OfficeNamespaces.TABLE_NS, "table-cell", attrs))
    {
      columnCounter++;
      final String numColSpanStr = (String) attrs.getAttribute(namespace, "number-columns-spanned");
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

      // if (span < initialColumnSpan)
      // {
      // // ColumnBoundary cbs[] = getBoundariesForTable(tableCounter);
      // // for (int i = 0; i < cbs.length; i++)
      // // {
      // // System.out.print(cbs[i].getBoundary() + " ");
      // // }
      // // System.out.println();
      //
      // Log.error("A cell cannot span less than the declared columns: Declared=" + initialColumnSpan + " Computed="
      // + span);
      // }

      // there's no point to create number-columns-spanned attributes if we only span 1 column
      if (span > 1)
      {
        attrs.setAttribute(namespace, "number-columns-spanned", "" + span);
      }
      // we must also generate "covered-table-cell" elements for each column spanned
      // but we'll do this in the endElement, after we close this "table-cell"
    }

    // All styles have to be processed or you will loose the paragraph-styles and inline text-styles.
    // ..
    performStyleProcessing(attrs);

    final AttributeList attrList = buildAttributeList(attrs);
    xmlWriter.writeTag(namespace, elementType, attrList, XmlWriter.OPEN);
    // System.out.println("elementType = " + elementType);
  }

  private void collectBoundaryForElement(final AttributeMap attrs)
  {
    if (ReportTargetUtil.isElementOfType(OfficeNamespaces.TABLE_NS, "table-columns", attrs))
    {
      // A table row resets the column counter.
      resetCurrentRowBoundaryMarker();
    }
    else if (ReportTargetUtil.isElementOfType(OfficeNamespaces.TABLE_NS, "table-column", attrs))
    {
      final String styleName = (String) attrs.getAttribute(OfficeNamespaces.TABLE_NS, "style-name");
      if (styleName == null)
      {
        // This should not happen, but if it does, we will ignore that cell.
        return;
      }

      final OfficeStyle style = getPredefinedStylesCollection().getStyle("table-column", styleName);
      if (style == null)
      {
        // Now this is very bad. It means that there is no style defined with the given name.
        return;
      }

      final Element tableColumnProperties = style.getTableColumnProperties();
      String widthStr = (String) tableColumnProperties.getAttribute("column-width");
      widthStr = widthStr.substring(0, widthStr.indexOf(getUnitsOfMeasure(widthStr)));
      addColumnWidthToRowBoundaryMarker(Float.parseFloat(widthStr));
      ColumnBoundary currentRowBoundary = new ColumnBoundary(getCurrentRowBoundaryMarker());
      final List columnBoundaryList = getColumnBoundaryList();
      final int idx = columnBoundaryList.indexOf(currentRowBoundary);
      if (idx == -1)
      {
        columnBoundaryList.add(currentRowBoundary);
      }
      else
      {
        currentRowBoundary = (ColumnBoundary) columnBoundaryList.get(idx);
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

  private void createTableColumns() throws ReportProcessingException
  {
    try
    {
      final XmlWriter xmlWriter = getXmlWriter();
      // at this point we need to generate the table-columns section based on our boundary table
      // <table-columns>
      // <table-column style-name="coX"/>
      // ..
      // </table-columns>
      // the first boundary is '0' which is a placeholder so we will ignore it
      xmlWriter.writeTag(OfficeNamespaces.TABLE_NS, "table-columns", null, XmlWriterSupport.OPEN);

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
        final OfficeStyle style = deriveStyle("table-column", ("co" + String.valueOf(i) + "_"));
        final Section tableColumnProperties = new Section();
        tableColumnProperties.setType("table-column-properties");
        tableColumnProperties.setNamespace(style.getNamespace());
        tableColumnProperties.setAttribute(style.getNamespace(), "column-width", String.valueOf(columnWidth)
            + getUnitsOfMeasure(null));
        style.addNode(tableColumnProperties);

        final AttributeList myAttrList = new AttributeList();
        myAttrList.setAttribute(OfficeNamespaces.TABLE_NS, "style-name", style.getStyleName());
        xmlWriter.writeTag(OfficeNamespaces.TABLE_NS, "table-column", myAttrList, XmlWriterSupport.CLOSE);
      }
      xmlWriter.writeCloseTag();
    }
    catch (IOException e)
    {
      throw new ReportProcessingException("Failed", e);
    }
  }

  protected void endOther(final AttributeMap attrs) throws DataSourceException, ReportProcessingException
  {
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
    // if this is the report namespace, write out a table definition ..
    if (OfficeNamespaces.TABLE_NS.equals(namespace)
        && ("table".equals(elementType) || "covered-table-cell".equals(elementType)
        || "table-column".equals(elementType) || "table-columns".equals(elementType)))
    {
      return;
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
      throw new ReportProcessingException("Failed", e);
    }
  }

  private void generateCoveredTableCells(final AttributeMap attrs) throws IOException
  {
    if (ReportTargetUtil.isElementOfType(OfficeNamespaces.TABLE_NS, "table-cell", attrs) == false)
    {
      return;
    }

    // do this after we close the tag
    final XmlWriter xmlWriter = getXmlWriter();
    final Object attribute = attrs.getAttribute(OfficeNamespaces.TABLE_NS, "number-columns-spanned");
    final int span = TextUtilities.parseInt((String) attribute, 0);
    for (int i = 1; i < span; i++)
    {
      xmlWriter.writeTag(OfficeNamespaces.TABLE_NS, "covered-table-cell", null, XmlWriter.CLOSE);
    }
  }

  public String getExportDescriptor()
  {
    return "raw/" + PentahoReportEngineMetaData.OPENDOCUMENT_SPREADSHEET;
  }

  // /////////////////////////////////////////////////////////////////////////
  public void processText(final String text) throws DataSourceException, ReportProcessingException
  {
    if (isRepeatingSection() || isElementBoundaryCollectionPass())
    {
      return;
    }
    super.processText(text);
  }

  public void processContent(final DataFlags value) throws DataSourceException, ReportProcessingException
  {
    if (isRepeatingSection() || isElementBoundaryCollectionPass())
    {
      return;
    }
    // System.out.println(value.getValue());
    super.processContent(value);
  }

  protected void startContent(final AttributeMap attrs) throws IOException, DataSourceException,
      ReportProcessingException
  {
    if (isElementBoundaryCollectionPass())
    {
      return;
    }

    final XmlWriter xmlWriter = getXmlWriter();
    xmlWriter.writeTag(OfficeNamespaces.OFFICE_NS, "spreadsheet", null, XmlWriterSupport.OPEN);

    final AttributeMap tableAttributes = new AttributeMap();
    tableAttributes.setAttribute(JFreeReportInfo.REPORT_NAMESPACE, Element.NAMESPACE_ATTRIBUTE, OfficeNamespaces.TABLE_NS);
    tableAttributes.setAttribute(JFreeReportInfo.REPORT_NAMESPACE, Element.TYPE_ATTRIBUTE, "table");
    tableAttributes.setAttribute(OfficeNamespaces.TABLE_NS, "style-name", generateInitialTableStyle());
    tableAttributes.setAttribute(OfficeNamespaces.TABLE_NS, "name", "Report");

    performStyleProcessing(tableAttributes);

    xmlWriter.writeTag(OfficeNamespaces.TABLE_NS, "table", buildAttributeList(tableAttributes), XmlWriterSupport.OPEN);
    createTableColumns();
  }

  private String generateInitialTableStyle () throws ReportProcessingException
  {
    final OfficeStylesCollection predefStyles = getPredefinedStylesCollection();
    final OfficeStyles commonStyles = predefStyles.getAutomaticStyles();
    if (commonStyles.containsStyle("table", "Initial_Table") == false)
    {
      final String masterPageName = createMasterPage();

      final OfficeStyle tableStyle = new OfficeStyle();
      tableStyle.setStyleFamily("table");
      tableStyle.setStyleName("Initial_Table");
      tableStyle.setAttribute(OfficeNamespaces.STYLE_NS, "master-page-name", masterPageName);
      final Element tableProperties = produceFirstChild(tableStyle, OfficeNamespaces.STYLE_NS, "table-properties");
      tableProperties.setAttribute(OfficeNamespaces.FO_NS, "background-color", "transparent");
      commonStyles.addStyle(tableStyle);
    }
    return "Initial_Table";
  }

  private String createMasterPage() throws ReportProcessingException
  {
    final OfficeStylesCollection predefStyles = getPredefinedStylesCollection();
    final MasterPageFactory masterPageFactory = new MasterPageFactory(predefStyles.getMasterStyles());
    final OfficeMasterPage masterPage;
    if (masterPageFactory.containsMasterPage("Standard", null, null) == false)
    {
      masterPage = masterPageFactory.createMasterPage("Standard", null, null);

      final CSSNumericValue zeroLength = CSSNumericValue.createValue(CSSNumericType.CM, 0);
      final String pageLayoutTemplate = masterPage.getPageLayout();
      if (pageLayoutTemplate == null)
      {
        // there is no pagelayout. Create one ..
        final String derivedLayout = masterPageFactory.createPageStyle
            (getGlobalStylesCollection().getAutomaticStyles(), zeroLength, zeroLength);
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

  protected void endContent(final AttributeMap attrs) throws IOException, DataSourceException,
      ReportProcessingException
  {
    // todo
    if (isElementBoundaryCollectionPass() == false)
    {
      final XmlWriter xmlWriter = getXmlWriter();
      xmlWriter.writeCloseTag();
      xmlWriter.writeCloseTag();
    }
  }

  public void endReport(final ReportStructureRoot report) throws DataSourceException, ReportProcessingException
  {
    super.endReport(report);
    setElementBoundaryCollectionPass(false);
    resetTableCounter();
    columnCounter = 0;
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
      sortedBoundaryArray = (ColumnBoundary[]) getColumnBoundaryList().toArray(EMPTY_COLBOUNDS);
      Arrays.sort(sortedBoundaryArray);
    }
    return sortedBoundaryArray;
  }

  private List getColumnBoundaryList()
  {
    return columnBoundaryList;
  }

  private void addColumnWidthToRowBoundaryMarker(final float width)
  {
    currentRowBoundaryMarker += width;
  }

  private float getCurrentRowBoundaryMarker()
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
      final List boundariesForTable = new ArrayList();
      final List boundaryList = getColumnBoundaryList();
      for (int i = 0; i < boundaryList.size(); i++)
      {
        final ColumnBoundary b = (ColumnBoundary) boundaryList.get(i);
        if (b.isContainedByTable(table))
        {
          boundariesForTable.add(b);
        }
      }
      boundariesForTableArray = (ColumnBoundary[]) boundariesForTable.toArray(EMPTY_COLBOUNDS);
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
    // System.out.println("table = " + table + " col = " + col + " rowBoundaries.length = " + tableBoundaries.length + "
    // cellWidth = " + cellWidth + " span = " + span);
    return span;
  }

  protected String getTargetMimeType()
  {
    return "application/vnd.oasis.opendocument.spreadsheet";
  }

}
