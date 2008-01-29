/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TextRawReportTarget.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 14:35:11 $
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

package com.sun.star.report.pentaho.output.text;

import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.util.Iterator;
import java.util.Map;

import com.sun.star.report.ImageService;
import com.sun.star.report.InputRepository;
import com.sun.star.report.OutputRepository;
import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.pentaho.PentahoReportEngineMetaData;
import com.sun.star.report.pentaho.model.OfficeMasterPage;
import com.sun.star.report.pentaho.model.OfficeMasterStyles;
import com.sun.star.report.pentaho.model.OfficeStyle;
import com.sun.star.report.pentaho.model.OfficeStyles;
import com.sun.star.report.pentaho.model.OfficeStylesCollection;
import com.sun.star.report.pentaho.model.PageSection;
import com.sun.star.report.pentaho.output.OfficeDocumentReportTarget;
import com.sun.star.report.pentaho.output.StyleUtilities;
import com.sun.star.report.pentaho.styles.LengthCalculator;
import org.jfree.layouting.input.style.values.CSSNumericValue;
import org.jfree.layouting.util.AttributeMap;
import org.jfree.report.DataSourceException;
import org.jfree.report.ReportProcessingException;
import org.jfree.report.flow.ReportJob;
import org.jfree.report.flow.ReportStructureRoot;
import org.jfree.report.flow.ReportTargetUtil;
import org.jfree.report.structure.Element;
import org.jfree.report.structure.Section;
import org.jfree.report.util.AttributeNameGenerator;
import org.jfree.report.util.IntegerCache;
import org.jfree.resourceloader.ResourceKey;
import org.jfree.resourceloader.ResourceManager;
import org.jfree.util.FastStack;
import org.jfree.util.ObjectUtilities;
//import org.jfree.util.Log;
import org.jfree.xmlns.common.AttributeList;
import org.jfree.xmlns.writer.XmlWriter;
import org.jfree.xmlns.writer.XmlWriterSupport;

/**
 * Creation-Date: 03.07.2006, 16:28:00
 *
 * @author Thomas Morgner
 */
public class TextRawReportTarget extends OfficeDocumentReportTarget
{
  private static final String VARIABLES_HIDDEN_STYLE_WITH_KEEPWNEXT = "variables_paragraph_with_next";
  private static final String VARIABLES_HIDDEN_STYLE_WITHOUT_KEEPWNEXT = "variables_paragraph_without_next";

  private static final int TABLE_LAYOUT_VARIABLES_PARAGRAPH = 0;
  private static final int TABLE_LAYOUT_VARIABLES_IN_FIRST_CELL = 1;
  private static final int TABLE_LAYOUT_SINGLE_DETAIL_TABLE = 2;

  private static final int CP_SETUP = 0;
  private static final int CP_FIRST_TABLE = 1;
  private static final int CP_NEXT_TABLE = 2;

  // This is the initial state of the detail-band processing. It states, that we are now waiting for a
  // detail-band to be printed.
  private static final int DETAIL_SECTION_WAIT = 0;
  // The first detail section has started.
  private static final int DETAIL_SECTION_FIRST_STARTED = 1;
  // The first detail section has been printed.
  private static final int DETAIL_SECTION_FIRST_PRINTED = 2;
  // An other detail section has started
  private static final int DETAIL_SECTION_OTHER_STARTED = 3;
  // The other detail section has been printed.
  private static final int DETAIL_SECTION_OTHER_PRINTED = 4;


  private boolean pageFooterOnReportFooter;
  private boolean pageFooterOnReportHeader;
  private boolean pageHeaderOnReportFooter;
  private boolean pageHeaderOnReportHeader;
  private int contentProcessingState;
  private OfficeMasterPage currentMasterPage;

  private FastStack activePageContext;
  private MasterPageFactory masterPageFactory;
  private LengthCalculator sectionHeight;
  private String variables;
  private PageBreakDefinition pageBreakDefinition;
  private VariablesDeclarations variablesDeclarations;
  private boolean columnBreakPending;
  private boolean sectionKeepTogether;
  private AttributeNameGenerator sectionNames;

  private int detailBandProcessingState;

  private int tableLayoutConfig;
  private int expectedTableRowCount;
  private boolean firstCellSeen;
  private boolean cellEmpty;

  public TextRawReportTarget(final ReportJob reportJob,
                             final ResourceManager resourceManager,
                             final ResourceKey baseResource,
                             final InputRepository inputRepository,
                             final OutputRepository outputRepository,
                             final String target,
                             final ImageService imageService)
      throws ReportProcessingException
  {
    super(reportJob, resourceManager, baseResource, inputRepository, outputRepository, target, imageService);
    activePageContext = new FastStack();
    this.sectionNames = new AttributeNameGenerator();

    this.tableLayoutConfig = TABLE_LAYOUT_SINGLE_DETAIL_TABLE;
  }

  protected String getTargetMimeType()
  {
    return "application/vnd.oasis.opendocument.text";
  }

  /**
   * Checks, whether a manual page break should be inserted at the next possible location.
   *
   * @return true, if a pagebreak is pending, false otherwise.
   */
  private boolean isPagebreakPending()
  {
    return pageBreakDefinition != null;
  }

  private boolean isResetPageNumber()
  {
    if (pageBreakDefinition == null)
    {
      return false;
    }
    return pageBreakDefinition.isResetPageNumber();
  }

  /**
   * Defines, whether a manual pagebreak should be inserted at the next possible location.
   *
   * @param pageBreakDefinition the new flag value.
   */
  private void setPagebreakDefinition(final PageBreakDefinition pageBreakDefinition)
  {
    this.pageBreakDefinition = pageBreakDefinition;
  }


  private PageBreakDefinition getPagebreakDefinition()
  {
    return pageBreakDefinition;
  }

  // todo
  private boolean isKeepTableWithNext()
  {
    final int keepTogetherState = getCurrentContext().getKeepTogether();
    if (keepTogetherState == PageContext.KEEP_TOGETHER_GROUP)
    {
      return true;
    }

    final boolean keepWithNext;
    if (keepTogetherState == PageContext.KEEP_TOGETHER_FIRST_DETAIL)
    {
      keepWithNext = (detailBandProcessingState == DETAIL_SECTION_WAIT);
    }
    else
    {
      keepWithNext = false;
    }
    return keepWithNext;
  }

  private boolean isSectionPagebreakAfter(final AttributeMap attrs)
  {
    final Object forceNewPage =
        attrs.getAttribute(OfficeNamespaces.OOREPORT_NS, "force-new-page");
    if ("after-section".equals(forceNewPage))
    {
      return true;
    }
    if ("before-after-section".equals(forceNewPage))
    {
      return true;
    }
    return false;
  }

  private boolean isSectionPagebreakBefore(final AttributeMap attrs)
  {
    final Object forceNewPage =
        attrs.getAttribute(OfficeNamespaces.OOREPORT_NS, "force-new-page");
    if ("before-section".equals(forceNewPage))
    {
      return true;
    }
    if ("before-after-section".equals(forceNewPage))
    {
      return true;
    }
    return false;
  }

  private PageContext getCurrentContext()
  {
    return (PageContext) activePageContext.peek();
  }

  private String createMasterPage(final boolean printHeader,
                                  final boolean printFooter)
      throws ReportProcessingException
  {
    // create the master page for the report-header.
    // If there is a page-header or footer in the report that gets
    // surpressed on the report-header, we have to insert a pagebreak
    // afterwards.

    String activePageFooter = null;
    // Check, whether the report header can have a page-header
    final PageContext context = getCurrentContext();
    if (printFooter)
    {
      activePageFooter = context.getPageFooterContent();
    }
    String activePageHeader = null;
    if (printHeader)
    {
      // we have to insert a manual pagebreak after the report header.
      activePageHeader = context.getPageHeaderContent();
    }

    final String masterPageName;
    if (currentMasterPage == null ||
        masterPageFactory.containsMasterPage("Standard", activePageHeader, activePageFooter) == false)
    {

      final CSSNumericValue headerSize = context.getAllHeaderSize();
      final CSSNumericValue footerSize = context.getAllFooterSize();


      currentMasterPage = masterPageFactory.createMasterPage("Standard", activePageHeader, activePageFooter);

//      Log.debug("Created a new master-page: " + currentMasterPage.getStyleName());

      // todo: Store the page-layouts as well.
      // The page layouts are derived from a common template, but as the
      // header-heights differ, we have to derive these beasts instead
      // of copying them

      final OfficeStylesCollection officeStylesCollection = getGlobalStylesCollection();
      final OfficeMasterStyles officeMasterStyles = officeStylesCollection.getMasterStyles();
      final String pageLayoutTemplate = currentMasterPage.getPageLayout();
      if (pageLayoutTemplate == null)
      {
        // there is no pagelayout. Create one ..
        final String derivedLayout = masterPageFactory.createPageStyle
            (getGlobalStylesCollection().getAutomaticStyles(), headerSize, footerSize);
        currentMasterPage.setPageLayout(derivedLayout);
      }
      else
      {
        final String derivedLayout = masterPageFactory.derivePageStyle(pageLayoutTemplate,
            getPredefinedStylesCollection().getAutomaticStyles(),
            getGlobalStylesCollection().getAutomaticStyles(), headerSize, footerSize);
        currentMasterPage.setPageLayout(derivedLayout);
      }
      officeMasterStyles.addMasterPage(currentMasterPage);
      masterPageName = currentMasterPage.getStyleName();
    }
    else
    {
      // retrieve the master-page.
      final OfficeMasterPage masterPage = masterPageFactory.getMasterPage("Standard", activePageHeader, activePageFooter);
      if (ObjectUtilities.equal(masterPage.getStyleName(), currentMasterPage.getStyleName()))
      {
        // They are the same,
        masterPageName = null;
      }
      else
      {
        // reuse the existing one ..
        currentMasterPage = masterPage;
        masterPageName = currentMasterPage.getStyleName();
      }
    }

    // if either the pageheader or footer are *not* printed with the
    // report header, then this implies that we have to insert a manual
    // pagebreak at the end of the section.

    if ((printHeader == false && context.getHeader() != null) ||
        (printFooter == false && context.getFooter() != null))
    {
      setPagebreakDefinition(new PageBreakDefinition(isResetPageNumber()));
    }

    return masterPageName;
  }

  private boolean isColumnBreakPending()
  {
    return columnBreakPending;
  }

  private void setColumnBreakPending(final boolean columnBreakPending)
  {
    this.columnBreakPending = columnBreakPending;
  }

  private Integer parseInt(final Object value)
  {
    if (value instanceof Number)
    {
      final Number n = (Number) value;
      return IntegerCache.getInteger(n.intValue());
    }
    if (value instanceof String)
    {
      try
      {
        return IntegerCache.getInteger(Integer.parseInt((String) value));
      }
      catch (NumberFormatException nfe)
      {
        return null;
      }
    }
    return null;
  }

  private BufferState applyColumnsToPageBand(final BufferState contents,
                                             final int numberOfColumns)
      throws IOException, ReportProcessingException
  {
    if (numberOfColumns <= 1)
    {
      return contents;
    }
    startBuffering(getGlobalStylesCollection(), true);
    // derive section style ..

    // This is a rather cheap solution to the problem. In a sane world, we would have to feed the
    // footer multiple times. Right now, we simply rely on the balacing, which should make sure that
    // the column's content are evenly distributed.
    final XmlWriter writer = getXmlWriter();
    final AttributeList attrs = new AttributeList();
    attrs.setAttribute(OfficeNamespaces.TEXT_NS, "style-name", generateSectionStyle(numberOfColumns));
    attrs.setAttribute(OfficeNamespaces.TEXT_NS, "name", sectionNames.generateName("Section"));
    writer.writeTag(OfficeNamespaces.TEXT_NS, "section", attrs, XmlWriterSupport.OPEN);
    for (int i = 0; i < numberOfColumns; i++)
    {
      writer.writeStream(contents.getXmlAsReader());
    }

    writer.writeCloseTag();
    return finishBuffering();
  }

  private String generateSectionStyle(final int columnCount)
  {
    final OfficeStyles automaticStyles = getStylesCollection().getAutomaticStyles();
    final String styleName = getAutoStyleNameGenerator().generateName("auto_section_style");

    final Section sectionProperties = new Section();
    sectionProperties.setNamespace(OfficeNamespaces.STYLE_NS);
    sectionProperties.setType("section-properties");
    sectionProperties.setAttribute(OfficeNamespaces.FO_NS, "background-color", "transparent");
    sectionProperties.setAttribute(OfficeNamespaces.TEXT_NS, "dont-balance-text-columns", "false");
    sectionProperties.setAttribute(OfficeNamespaces.STYLE_NS, "editable", "false");

    if (columnCount > 1)
    {
      final Section columns = new Section();
      columns.setNamespace(OfficeNamespaces.STYLE_NS);
      columns.setType("columns");
      columns.setAttribute(OfficeNamespaces.FO_NS, "column-count", String.valueOf(columnCount));
      columns.setAttribute(OfficeNamespaces.STYLE_NS, "column-gap", "0cm");
      sectionProperties.addNode(columns);

//    final Section columnSep = new Section();
//    columnSep.setNamespace(OfficeNamespaces.STYLE_NS);
//    columnSep.setType("column-sep");
//    columnSep.setAttribute(OfficeNamespaces.STYLE_NS, "width", "0.035cm");
//    columnSep.setAttribute(OfficeNamespaces.STYLE_NS, "color", "#000000");
//    columnSep.setAttribute(OfficeNamespaces.STYLE_NS, "height", "100%");
//    columns.addNode(columnSep);

      for (int i = 0; i < columnCount; i++)
      {
        final Section column = new Section();
        column.setNamespace(OfficeNamespaces.STYLE_NS);
        column.setType("column");
        column.setAttribute(OfficeNamespaces.STYLE_NS, "rel-width", "1*");
        column.setAttribute(OfficeNamespaces.FO_NS, "start-indent", "0cm");
        column.setAttribute(OfficeNamespaces.FO_NS, "end-indent", "0cm");
        columns.addNode(column);
      }
    }

    final OfficeStyle style = new OfficeStyle();
    style.setNamespace(OfficeNamespaces.STYLE_NS);
    style.setType("style");
    style.setAttribute(OfficeNamespaces.STYLE_NS, "name", styleName);
    style.setAttribute(OfficeNamespaces.STYLE_NS, "family", "section");
    style.addNode(sectionProperties);

    automaticStyles.addStyle(style);
    return styleName;
  }


  /**
   * Starts the output of a new office document. This method writes the generic 'office:document-content' tag along with
   * all known namespace declarations.
   *
   * @param report the report object.
   * @throws org.jfree.report.DataSourceException
   *          if there was an error accessing the datasource
   * @throws org.jfree.report.ReportProcessingException
   *          if some other error occured.
   */
  public void startReport(final ReportStructureRoot report)
      throws DataSourceException, ReportProcessingException
  {
    super.startReport(report);
    variablesDeclarations = new VariablesDeclarations();
    detailBandProcessingState = DETAIL_SECTION_WAIT;
    sectionNames.reset();

    pageFooterOnReportFooter = false;
    pageFooterOnReportHeader = false;
    pageHeaderOnReportFooter = false;
    pageHeaderOnReportHeader = false;
    contentProcessingState = TextRawReportTarget.CP_SETUP;

    activePageContext.clear();
    activePageContext.push(new PageContext());

    final OfficeStylesCollection predefStyles = getPredefinedStylesCollection();
    masterPageFactory = new MasterPageFactory(predefStyles.getMasterStyles());

    predefStyles.getAutomaticStyles().addStyle(createVariablesStyle(true));
    predefStyles.getAutomaticStyles().addStyle(createVariablesStyle(false));
  }

  private OfficeStyle createVariablesStyle(final boolean keepWithNext)
  {
    final OfficeStyle variablesSectionStyle = new OfficeStyle();
    variablesSectionStyle.setStyleFamily("paragraph");
    if (keepWithNext)
    {
      variablesSectionStyle.setStyleName(TextRawReportTarget.VARIABLES_HIDDEN_STYLE_WITH_KEEPWNEXT);
    }
    else
    {
      variablesSectionStyle.setStyleName(TextRawReportTarget.VARIABLES_HIDDEN_STYLE_WITHOUT_KEEPWNEXT);
    }

    final Section paragraphProps = new Section();
    paragraphProps.setNamespace(OfficeNamespaces.STYLE_NS);
    paragraphProps.setType("paragraph-properties");
    paragraphProps.setAttribute(OfficeNamespaces.FO_NS, "background-color", "transparent");
    paragraphProps.setAttribute(OfficeNamespaces.FO_NS, "text-align", "start");
    paragraphProps.setAttribute(OfficeNamespaces.FO_NS, "keep-with-next", "always");
    paragraphProps.setAttribute(OfficeNamespaces.FO_NS, "keep-together", "always");
    paragraphProps.setAttribute(OfficeNamespaces.STYLE_NS, "vertical-align", "top");
    variablesSectionStyle.addNode(paragraphProps);

    final Section textProps = new Section();
    textProps.setNamespace(OfficeNamespaces.STYLE_NS);
    textProps.setType("text-properties");
    textProps.setAttribute(OfficeNamespaces.FO_NS, "font-variant", "normal");
    textProps.setAttribute(OfficeNamespaces.FO_NS, "text-transform", "none");
    textProps.setAttribute(OfficeNamespaces.FO_NS, "color", "#ffffff");
    textProps.setAttribute(OfficeNamespaces.STYLE_NS, "text-outline", "false");
    textProps.setAttribute(OfficeNamespaces.STYLE_NS, "text-blinking", "false");
    textProps.setAttribute(OfficeNamespaces.STYLE_NS, "text-line-through-style", "none");
    textProps.setAttribute(OfficeNamespaces.STYLE_NS, "text-line-through-mode", "continuous");
    textProps.setAttribute(OfficeNamespaces.STYLE_NS, "text-position", "0% 100%");
    textProps.setAttribute(OfficeNamespaces.STYLE_NS, "font-name", "Tahoma");
    textProps.setAttribute(OfficeNamespaces.FO_NS, "font-size", "1pt");
    textProps.setAttribute(OfficeNamespaces.FO_NS, "letter-spacing", "normal");
    textProps.setAttribute(OfficeNamespaces.STYLE_NS, "letter-kerning", "false");
    textProps.setAttribute(OfficeNamespaces.FO_NS, "font-style", "normal");
    textProps.setAttribute(OfficeNamespaces.FO_NS, "text-shadow", "none");
    textProps.setAttribute(OfficeNamespaces.STYLE_NS, "text-underline-style", "none");
    textProps.setAttribute(OfficeNamespaces.STYLE_NS, "text-underline-mode", "continuous");
    textProps.setAttribute(OfficeNamespaces.FO_NS, "font-weight", "normal");
    textProps.setAttribute(OfficeNamespaces.STYLE_NS, "text-rotation-angle", "0");
    textProps.setAttribute(OfficeNamespaces.STYLE_NS, "text-emphasize", "none");
    textProps.setAttribute(OfficeNamespaces.STYLE_NS, "text-combine", "none");
    textProps.setAttribute(OfficeNamespaces.STYLE_NS, "text-combine-start-char", "");
    textProps.setAttribute(OfficeNamespaces.STYLE_NS, "text-combine-end-char", "");
    textProps.setAttribute(OfficeNamespaces.STYLE_NS, "text-blinking", "false");
    textProps.setAttribute(OfficeNamespaces.STYLE_NS, "text-scale", "100%");
    textProps.setAttribute(OfficeNamespaces.STYLE_NS, "font-relief", "none");
    textProps.setAttribute(OfficeNamespaces.STYLE_NS, "text-display", "none");
    variablesSectionStyle.addNode(textProps);
    return variablesSectionStyle;
  }

  protected void startContent(final AttributeMap attrs)
      throws IOException, DataSourceException, ReportProcessingException
  {
    final XmlWriter xmlWriter = getXmlWriter();
    xmlWriter.writeTag(OfficeNamespaces.OFFICE_NS, "text", null, XmlWriterSupport.OPEN);

    // now start the buffering. We have to insert the variables declaration
    // later ..
    startBuffering(getStylesCollection(), true);

    final PageContext pageContext = getCurrentContext();
    final Object columnCountRaw = attrs.getAttribute(OfficeNamespaces.FO_NS, "column-count");
    final Integer colCount = parseInt(columnCountRaw);
    if (colCount != null)
    {
      pageContext.setColumnCount(colCount);
    }

  }

  protected void startOther(final AttributeMap attrs)
      throws IOException, DataSourceException, ReportProcessingException
  {
    final String namespace = ReportTargetUtil.getNamespaceFromAttribute(attrs);
    final String elementType = ReportTargetUtil.getElemenTypeFromAttribute(attrs);

    if (ReportTargetUtil.isElementOfType(OfficeNamespaces.INTERNAL_NS, "image", attrs))
    {
      startImageProcessing(attrs);
      return;
    }

    if (isFilteredNamespace(namespace))
    {
      throw new IllegalStateException("This element should be hidden: " +
          namespace + ", " + elementType);
    }

    if (isTableMergeActive() && detailBandProcessingState == DETAIL_SECTION_OTHER_PRINTED)
    {
      // Skip the columns section if the tables get merged..
      if (ReportTargetUtil.isElementOfType(OfficeNamespaces.TABLE_NS, "table-columns", attrs))
      {
        startBuffering(getStylesCollection(), true);
        return;
      }
    }

    openSection();

    if (ReportTargetUtil.isElementOfType(OfficeNamespaces.TABLE_NS, "table", attrs))
    {
      startTable(attrs);
      return;
    }

    final XmlWriter xmlWriter = getXmlWriter();
    if (ReportTargetUtil.isElementOfType(OfficeNamespaces.TABLE_NS, "table-row", attrs))
    {
      startRow(attrs);
      return;
    }

    if (ReportTargetUtil.isElementOfType(OfficeNamespaces.TEXT_NS, "variable-set", attrs))
    {
      // update the variables-declaration thingie ..
      final String varName = (String) attrs.getAttribute(OfficeNamespaces.TEXT_NS, "name");
      final String varType = (String) attrs.getAttribute(OfficeNamespaces.OFFICE_NS, "value-type");
      final String newVarName = variablesDeclarations.produceVariable(varName, varType);
      attrs.setAttribute(OfficeNamespaces.TEXT_NS, "name", newVarName);
    }
    else if (ReportTargetUtil.isElementOfType(OfficeNamespaces.TEXT_NS, "variable-get", attrs))
    {
      final String varName = (String) attrs.getAttribute(OfficeNamespaces.TEXT_NS, "name");
      final String varType = (String) attrs.getAttribute(OfficeNamespaces.OFFICE_NS, "value-type");
      final String newVarName = variablesDeclarations.produceVariable(varName, varType);
      attrs.setAttribute(OfficeNamespaces.TEXT_NS, "name", newVarName);
      // this one must not be written, as the DTD does not declare it.
      // attrs.setAttribute(OfficeNamespaces.OFFICE_NS, "value-type", null);
    }

    if (tableLayoutConfig == TABLE_LAYOUT_VARIABLES_PARAGRAPH && variables != null)
    {
      // This cannot happen as long as the report sections only contain tables. But at some point in the
      // future they will be made of paragraphs, and then we are prepared ..
      // Log.debug("Variables-Section in own paragraph " + variables);

      StyleUtilities.copyStyle("paragraph",
          TextRawReportTarget.VARIABLES_HIDDEN_STYLE_WITH_KEEPWNEXT, getStylesCollection(),
          getGlobalStylesCollection(), getPredefinedStylesCollection());
      xmlWriter.writeTag(OfficeNamespaces.TEXT_NS, "p", "style-name",
          TextRawReportTarget.VARIABLES_HIDDEN_STYLE_WITH_KEEPWNEXT, XmlWriterSupport.OPEN);
      xmlWriter.writeText(variables);
      xmlWriter.writeCloseTag();
      variables = null;
    }

    if (ReportTargetUtil.isElementOfType(OfficeNamespaces.TABLE_NS, "table-cell", attrs))
    {
      cellEmpty = true;
    }

    boolean keepTogetherOnParagraph = true;

    if (keepTogetherOnParagraph)
    {
      if (ReportTargetUtil.isElementOfType(OfficeNamespaces.TEXT_NS, "p", attrs))
      {
        cellEmpty = false;
        if (firstCellSeen == false && sectionKeepTogether)
        {
          final String styleName = (String) attrs.getAttribute(OfficeNamespaces.TEXT_NS, "style-name");
          final OfficeStyle style = deriveStyle("paragraph", styleName);
          // Lets set the 'keep-together' flag..

          Element paragraphProps = style.getParagraphProperties();
          if (paragraphProps == null)
          {
            paragraphProps = new Section();
            paragraphProps.setNamespace(OfficeNamespaces.STYLE_NS);
            paragraphProps.setType("paragraph-properties");
            style.addNode(paragraphProps);
          }
          paragraphProps.setAttribute(OfficeNamespaces.FO_NS, "keep-together", "always");
          final int keepTogetherState = getCurrentContext().getKeepTogether();
          // We prevent pagebreaks within the two adjacent rows (this one and the next one) if
          // either a group-wide keep-together is defined or if we haven't reached the end of the
          // current section yet.
          if (keepTogetherState == PageContext.KEEP_TOGETHER_GROUP || expectedTableRowCount > 0)
          {
            paragraphProps.setAttribute(OfficeNamespaces.FO_NS, "keep-with-next", "always");
          }

          attrs.setAttribute(OfficeNamespaces.TEXT_NS, "style-name", style.getStyleName());
        }
      }
    }
    else
    {
      if (ReportTargetUtil.isElementOfType(OfficeNamespaces.TABLE_NS, "table-cell", attrs))
      {
        cellEmpty = false;
        if (firstCellSeen == false && sectionKeepTogether)
        {
          final String styleName = (String) attrs.getAttribute(OfficeNamespaces.TABLE_NS, "style-name");
          final OfficeStyle style = deriveStyle("table-cell", styleName);
          // Lets set the 'keep-together' flag..

          Element paragraphProps = style.getParagraphProperties();
          if (paragraphProps == null)
          {
            paragraphProps = new Section();
            paragraphProps.setNamespace(OfficeNamespaces.STYLE_NS);
            paragraphProps.setType("paragraph-properties");
            style.addNode(paragraphProps);
          }
          paragraphProps.setAttribute(OfficeNamespaces.FO_NS, "keep-together", "always");
          final int keepTogetherState = getCurrentContext().getKeepTogether();
          // We prevent pagebreaks within the two adjacent rows (this one and the next one) if
          // either a group-wide keep-together is defined or if we haven't reached the end of the
          // current section yet.
          if (keepTogetherState == PageContext.KEEP_TOGETHER_GROUP || expectedTableRowCount > 0)
          {
            paragraphProps.setAttribute(OfficeNamespaces.FO_NS, "keep-with-next", "always");
          }

          attrs.setAttribute(OfficeNamespaces.TABLE_NS, "style-name", style.getStyleName());
        }
      }
    }

    // process the styles as usual
    performStyleProcessing(attrs);

    final AttributeList attrList = buildAttributeList(attrs);
    xmlWriter.writeTag(namespace, elementType, attrList, XmlWriterSupport.OPEN);

    if (ReportTargetUtil.isElementOfType(OfficeNamespaces.TEXT_NS, "p", attrs))
    {
      cellEmpty = false;
      if (tableLayoutConfig != TABLE_LAYOUT_VARIABLES_PARAGRAPH)
      {
        if (variables != null)
        {
          //Log.debug("Variables-Section in existing cell " + variables);
          xmlWriter.writeText(variables);
          variables = null;
        }
      }
    }

  }

  private void startRow(final AttributeMap attrs)
      throws IOException, ReportProcessingException
  {
    firstCellSeen = false;
    expectedTableRowCount -= 1;
    final String rowStyle = (String) attrs.getAttribute(OfficeNamespaces.TABLE_NS, "style-name");
    final CSSNumericValue rowHeight = computeRowHeight(rowStyle);
    // Log.debug("Adding row-Style: " + rowStyle + " " + rowHeight);
    sectionHeight.add(rowHeight);

//    if (expectedTableRowCount > 0)
//    {
//      // Some other row. Create a keep-together
//
//    }
//    else
//    {
//      // This is the last row before the section will end.
//      // or (in some weird cases) There is no information when the row will end.
//      // Anyway, if we are here, we do not create a keep-together style on the table-row ..
//    }
    // process the styles as usual
    performStyleProcessing(attrs);

    final AttributeList attrList = buildAttributeList(attrs);
    getXmlWriter().writeTag(OfficeNamespaces.TABLE_NS, "table-row", attrList, XmlWriterSupport.OPEN);
  }

  private void startTable(final AttributeMap attrs)
      throws ReportProcessingException, IOException
  {
    final Integer trc = (Integer) attrs.getAttribute(OfficeNamespaces.INTERNAL_NS, "table-row-count");
    if (trc == null)
    {
      expectedTableRowCount = -1;
    }
    else
    {
      expectedTableRowCount = trc.intValue();
    }

    if (isSectionPagebreakBefore(attrs))
    {
      // force a pagebreak ..
      setPagebreakDefinition(new PageBreakDefinition(isResetPageNumber()));
    }

    // its a table. This means, it is a root-level element
    PageBreakDefinition breakDefinition = null;
    String masterPageName = null;
    final int currentRole = getCurrentRole();
    if (contentProcessingState == TextRawReportTarget.CP_FIRST_TABLE)
    {
      contentProcessingState = TextRawReportTarget.CP_NEXT_TABLE;

      // Processing the report header now.
      if (currentRole == OfficeDocumentReportTarget.ROLE_REPORT_HEADER)
      {
        breakDefinition = new PageBreakDefinition(isResetPageNumber());
        masterPageName = createMasterPage(pageHeaderOnReportHeader, pageFooterOnReportHeader);
        if (masterPageName == null)
        {
          // we should always have a master-page ...
          masterPageName = currentMasterPage.getStyleName();
        }
      }
      else if (currentRole == OfficeDocumentReportTarget.ROLE_REPORT_FOOTER)
      {
        breakDefinition = new PageBreakDefinition(isResetPageNumber());
        masterPageName = createMasterPage (pageHeaderOnReportFooter, pageFooterOnReportFooter);
        if (masterPageName == null && isSectionPagebreakBefore(attrs))
        {
          // If we have a manual pagebreak, then activate the current master-page again.
          masterPageName = currentMasterPage.getStyleName();
        }
        // But we skip this (and therefore the resulting pagebreak) if there is no manual break
        // and no other condition that would force an break.
      }
      else if (currentRole == OfficeDocumentReportTarget.ROLE_REPEATING_GROUP_HEADER)
      {
        // no pagebreaks ..
      }
      else if (currentRole == OfficeDocumentReportTarget.ROLE_REPEATING_GROUP_FOOTER)
      {
        // no pagebreaks ..
      }
      else if (currentMasterPage == null ||
               isPagebreakPending())
      {
        // Must be the first table, as we have no master-page yet.
        masterPageName = createMasterPage(true, true);
        setPagebreakDefinition(null);
        if (masterPageName == null)
        {
          // we should always have a master-page ...
          masterPageName = currentMasterPage.getStyleName();
        }
        breakDefinition = new PageBreakDefinition(isResetPageNumber());
      }
    }
    else if (isPagebreakPending() &&
        currentRole != OfficeDocumentReportTarget.ROLE_REPEATING_GROUP_HEADER &&
        currentRole != OfficeDocumentReportTarget.ROLE_REPEATING_GROUP_FOOTER)
    {
      // Derive an automatic style for the pagebreak.
//      Log.debug("Manual pagebreak (within the section): " + getCurrentRole());
      breakDefinition = getPagebreakDefinition();
      setPagebreakDefinition(null);
      masterPageName = createMasterPage(true, true);
      if (masterPageName == null || isSectionPagebreakBefore(attrs))
      {
        // If we have a manual pagebreak, then activate the current master-page again.
        masterPageName = currentMasterPage.getStyleName();
      }
    }

    final XmlWriter xmlWriter = getXmlWriter();
    if (detailBandProcessingState == DETAIL_SECTION_OTHER_PRINTED &&
        masterPageName != null)
    {
      // close the last table-tag, we will open a new one
      xmlWriter.writeCloseTag();
      // Reset the detail-state to 'started' so that the table's columns get printed now.
      detailBandProcessingState = DETAIL_SECTION_OTHER_STARTED;
    }

    if (tableLayoutConfig == TABLE_LAYOUT_VARIABLES_PARAGRAPH && variables != null)
    {
      if (masterPageName != null)
      {
        // write a paragraph that uses the VARIABLES_HIDDEN_STYLE as
        // primary style. Derive that one and add the manual pagebreak.
        // The predefined style already has the 'keep-together' flags set.
//        Log.debug("Variables-Section with new Master-Page " + variables + " " + masterPageName);

        final OfficeStyle style = deriveStyle("paragraph", TextRawReportTarget.VARIABLES_HIDDEN_STYLE_WITH_KEEPWNEXT);
        style.setAttribute(OfficeNamespaces.STYLE_NS, "master-page-name", masterPageName);
        if (breakDefinition.isResetPageNumber())
        {
          final Element paragraphProps = produceFirstChild(style, OfficeNamespaces.STYLE_NS, "paragraph-properties");
          paragraphProps.setAttribute(OfficeNamespaces.STYLE_NS, "page-number", "1");
        }
        if (isColumnBreakPending())
        {
          final Element paragraphProps = produceFirstChild(style, OfficeNamespaces.STYLE_NS, "paragraph-properties");
          paragraphProps.setAttribute(OfficeNamespaces.FO_NS, "break-before", "column");
          setColumnBreakPending(false);
        }
        xmlWriter.writeTag(OfficeNamespaces.TEXT_NS, "p", "style-name", style.getStyleName(), XmlWriterSupport.OPEN);
        xmlWriter.writeText(variables);
        xmlWriter.writeCloseTag();
        variables = null;
        masterPageName = null;
        breakDefinition = null;
      }
      else if (isColumnBreakPending())
      {
        setColumnBreakPending(false);

        final OfficeStyle style = deriveStyle("paragraph", TextRawReportTarget.VARIABLES_HIDDEN_STYLE_WITH_KEEPWNEXT);
        final Element paragraphProps = produceFirstChild(style, OfficeNamespaces.STYLE_NS, "paragraph-properties");
        paragraphProps.setAttribute(OfficeNamespaces.STYLE_NS, "page-number", "1");

        xmlWriter.writeTag(OfficeNamespaces.TEXT_NS, "p", "style-name", style.getStyleName(), XmlWriterSupport.OPEN);
        xmlWriter.writeText(variables);
        xmlWriter.writeCloseTag();
        variables = null;
      }
      else
      {
        // Write a paragraph without adding the pagebreak. We can reuse the global style, but we have to make
        // sure that the style is part of the current 'auto-style' collection.
//        Log.debug("Variables-Section " + variables);

        StyleUtilities.copyStyle("paragraph",
            TextRawReportTarget.VARIABLES_HIDDEN_STYLE_WITH_KEEPWNEXT, getStylesCollection(),
            getGlobalStylesCollection(), getPredefinedStylesCollection());
        xmlWriter.writeTag(OfficeNamespaces.TEXT_NS, "p", "style-name",
            TextRawReportTarget.VARIABLES_HIDDEN_STYLE_WITH_KEEPWNEXT, XmlWriterSupport.OPEN);
        xmlWriter.writeText(variables);
        xmlWriter.writeCloseTag();
        variables = null;
      }
    }

    final boolean keepWithNext = isKeepTableWithNext();
    final boolean localKeepTogether = "true".equals
        (attrs.getAttribute(OfficeNamespaces.OOREPORT_NS, "keep-together"));
    final boolean tableMergeActive = isTableMergeActive();
    if (tableMergeActive == false)
    {
      this.sectionKeepTogether = false;
    }
    else
    {
      this.sectionKeepTogether = localKeepTogether;
    }

    // Check, whether we have a reason to derive a style...
    if (masterPageName != null ||
        (tableMergeActive == false && (localKeepTogether || keepWithNext)) || isColumnBreakPending())
    {
      final String styleName = (String)
          attrs.getAttribute(OfficeNamespaces.TABLE_NS, "style-name");
      final OfficeStyle style = deriveStyle("table", styleName);

      if (masterPageName != null)
      {
//        Log.debug("Starting a new MasterPage: " + masterPageName);
        // Patch the current styles.
        // This usually only happens on Table-Styles or Paragraph-Styles
        style.setAttribute(OfficeNamespaces.STYLE_NS, "master-page-name", masterPageName);
        if (breakDefinition.isResetPageNumber())
        {
          final Element paragraphProps = produceFirstChild(style, OfficeNamespaces.STYLE_NS, "paragraph-properties");
          paragraphProps.setAttribute(OfficeNamespaces.STYLE_NS, "page-number", "1");
        }
      }
      if (isColumnBreakPending())
      {
        final Element paragraphProps = produceFirstChild(style, OfficeNamespaces.STYLE_NS, "paragraph-properties");
        paragraphProps.setAttribute(OfficeNamespaces.FO_NS, "break-before", "column");
        setColumnBreakPending(false);
      }

      // Inhibit breaks inside the table only if it has been defined and if we do not create one single
      // big detail section. In that case, this flag would be invalid and would cause layout-errors.
      if (tableMergeActive == false)
      {
        if (localKeepTogether)
        {
          final Element tableProps = produceFirstChild(style, OfficeNamespaces.STYLE_NS, "table-properties");
          tableProps.setAttribute(OfficeNamespaces.STYLE_NS, "may-break-between-rows", "false");
        }
      }
      else
      {
        if (detailBandProcessingState == DETAIL_SECTION_WAIT)
        {
          detailBandProcessingState = DETAIL_SECTION_FIRST_STARTED;
        }
        else if (detailBandProcessingState == DETAIL_SECTION_FIRST_PRINTED)
        {
          detailBandProcessingState = DETAIL_SECTION_OTHER_STARTED;
        }
      }
      if (keepWithNext)
      {
        final Element tableProps = produceFirstChild(style, OfficeNamespaces.STYLE_NS, "table-properties");
        tableProps.setAttribute(OfficeNamespaces.FO_NS, "keep-with-next", "always");
        // A keep-with-next does not work, if the may-break-betweek rows is not set to false ..
        tableProps.setAttribute(OfficeNamespaces.STYLE_NS, "may-break-between-rows", "false");
      }
      attrs.setAttribute(OfficeNamespaces.TABLE_NS, "style-name", style.getStyleName());
      // no need to copy the styles, this was done while deriving the
      // style ..
    }
    else
    {
      // Check, whether we may be able to skip the table.
      if (tableMergeActive)
      {
        if (detailBandProcessingState == DETAIL_SECTION_OTHER_PRINTED)
        {
          // Skip the whole thing ..
          return;
        }
        else if (detailBandProcessingState == DETAIL_SECTION_WAIT)
        {
          detailBandProcessingState = DETAIL_SECTION_FIRST_STARTED;
        }
        else if (detailBandProcessingState == DETAIL_SECTION_FIRST_PRINTED)
        {
          detailBandProcessingState = DETAIL_SECTION_OTHER_STARTED;
        }
      }

      // process the styles as usual
      performStyleProcessing(attrs);
    }

    final String namespace = ReportTargetUtil.getNamespaceFromAttribute(attrs);
    final String elementType = ReportTargetUtil.getElemenTypeFromAttribute(attrs);
    final AttributeList attrList = buildAttributeList(attrs);
    xmlWriter.writeTag(namespace, elementType, attrList, XmlWriterSupport.OPEN);
  }

  private boolean isTableMergeActive()
  {
    return getCurrentRole() == ROLE_DETAIL &&
        tableLayoutConfig == TABLE_LAYOUT_SINGLE_DETAIL_TABLE;
  }


  private void openSection()
      throws IOException
  {
    if (isRepeatingSection())
    {
      // repeating sections have other ways of defining columns ..
      return;
    }
    if (getCurrentRole() == ROLE_TEMPLATE ||
        getCurrentRole() == ROLE_SPREADSHEET_PAGE_HEADER ||
        getCurrentRole() == ROLE_SPREADSHEET_PAGE_FOOTER)
    {
      // the template section would break the multi-column stuff and we dont open up sections there
      // anyway ..
      return;
    }

    final PageContext pageContext = getCurrentContext();
    final Integer columnCount = pageContext.getColumnCount();
    if (columnCount != null)
    {
      if (pageContext.isSectionOpen() == false)
      {
        final AttributeList attrs = new AttributeList();
        attrs.setAttribute(OfficeNamespaces.TEXT_NS, "style-name", generateSectionStyle(columnCount.intValue()));
        attrs.setAttribute(OfficeNamespaces.TEXT_NS, "name", sectionNames.generateName("Section"));
        getXmlWriter().writeTag(OfficeNamespaces.TEXT_NS, "section", attrs, XmlWriterSupport.OPEN);

        pageContext.setSectionOpen(true);
      }
    }

  }

  protected void startReportSection(final AttributeMap attrs, final int role)
      throws IOException, DataSourceException, ReportProcessingException
  {
    sectionHeight = new LengthCalculator();
    if (role == OfficeDocumentReportTarget.ROLE_TEMPLATE ||
        role == OfficeDocumentReportTarget.ROLE_SPREADSHEET_PAGE_HEADER ||
        role == OfficeDocumentReportTarget.ROLE_SPREADSHEET_PAGE_FOOTER)
    {
      // Start buffering with an dummy styles-collection, so that the global styles dont get polluted ..
      startBuffering(new OfficeStylesCollection(), true);
      return;
    }

    if (role == OfficeDocumentReportTarget.ROLE_PAGE_HEADER)
    {
      startBuffering(getGlobalStylesCollection(), true);
      pageHeaderOnReportHeader = PageSection.isPrintWithReportHeader(attrs);
      pageHeaderOnReportFooter = PageSection.isPrintWithReportFooter(attrs);
    }
    else if (role == OfficeDocumentReportTarget.ROLE_PAGE_FOOTER)
    {
      startBuffering(getGlobalStylesCollection(), true);
      pageFooterOnReportHeader = PageSection.isPrintWithReportHeader(attrs);
      pageFooterOnReportFooter = PageSection.isPrintWithReportFooter(attrs);
    }
    else if (role == OfficeDocumentReportTarget.ROLE_REPEATING_GROUP_HEADER)
    {
      startBuffering(getGlobalStylesCollection(), true);
    }
    else if (role == OfficeDocumentReportTarget.ROLE_REPEATING_GROUP_FOOTER)
    {
      startBuffering(getGlobalStylesCollection(), true);
    }
    else if (role == OfficeDocumentReportTarget.ROLE_VARIABLES)
    {
      startBuffering(getGlobalStylesCollection(), false);
    }
    else
    {
      contentProcessingState = TextRawReportTarget.CP_FIRST_TABLE;
      if (role == OfficeDocumentReportTarget.ROLE_GROUP_HEADER)
      {
        // if we have a repeating header, then skip the first one ..
        startBuffering(getContentStylesCollection(), true);
      }
      else if (role == OfficeDocumentReportTarget.ROLE_GROUP_FOOTER)
      {
        // if this is a repeating footer, skip the last one. This means,
        // we have to buffer all group footers and wait for the next section..
        startBuffering(getContentStylesCollection(), true);
      }

      if (role != OfficeDocumentReportTarget.ROLE_DETAIL)
      {
        // reset the detail-state. The flag will be updated on startTable and endOther(Table) if the
        // current role is ROLE_DETAIL
        detailBandProcessingState = DETAIL_SECTION_WAIT;
      }
    }
  }


  protected void startGroup(final AttributeMap attrs)
      throws IOException, DataSourceException, ReportProcessingException
  {
    super.startGroup(attrs);
    final PageContext pageContext = new PageContext(getCurrentContext());
    activePageContext.push(pageContext);

    final Object resetPageNumber = attrs.getAttribute(OfficeNamespaces.OOREPORT_NS, "reset-page-number");
    if ("true".equals(resetPageNumber))
    {
      setPagebreakDefinition(new PageBreakDefinition(true));
    }

    final Object keepTogether = attrs.getAttribute(OfficeNamespaces.OOREPORT_NS, "keep-together");
    if ("whole-group".equals(keepTogether))
    {
      pageContext.setKeepTogether(PageContext.KEEP_TOGETHER_GROUP);
    }
    else if ("with-first-detail".equals(keepTogether))
    {
      if (pageContext.getKeepTogether() != PageContext.KEEP_TOGETHER_GROUP)
      {
        pageContext.setKeepTogether(PageContext.KEEP_TOGETHER_FIRST_DETAIL);
      }
    }

    final Object columnCountRaw = attrs.getAttribute(OfficeNamespaces.FO_NS, "column-count");
    final Integer colCount = parseInt(columnCountRaw);
    if (colCount != null)
    {
      pageContext.setColumnCount(colCount);
    }

    final Object newColumn = attrs.getAttribute(OfficeNamespaces.OOREPORT_NS, "start-new-column");
    if ("true".equals(newColumn))
    {
      setColumnBreakPending(true);
    }
  }

  protected void startGroupInstance(final AttributeMap attrs)
      throws IOException, DataSourceException, ReportProcessingException
  {
    if (getGroupContext().isGroupWithRepeatingSection())
    {
      setPagebreakDefinition(new PageBreakDefinition(isResetPageNumber()));
    }
  }

  protected void endGroup(final AttributeMap attrs)
      throws IOException, DataSourceException, ReportProcessingException
  {
    if (getGroupContext().isGroupWithRepeatingSection())
    {
      setPagebreakDefinition(new PageBreakDefinition(isResetPageNumber()));
    }

    super.endGroup(attrs);
    finishSection();

    activePageContext.pop();
  }

  private void finishSection()
      throws ReportProcessingException
  {
    final PageContext pageContext = getCurrentContext();
    if (pageContext.isSectionOpen())
    {
      pageContext.setSectionOpen(false);
      try
      {
        getXmlWriter().writeCloseTag();
      }
      catch (IOException e)
      {
        throw new ReportProcessingException("IOError", e);
      }
    }
  }

  protected void endReportSection(final AttributeMap attrs, final int role)
      throws IOException, DataSourceException, ReportProcessingException
  {
    if (role == OfficeDocumentReportTarget.ROLE_TEMPLATE ||
        role == OfficeDocumentReportTarget.ROLE_SPREADSHEET_PAGE_HEADER ||
        role == OfficeDocumentReportTarget.ROLE_SPREADSHEET_PAGE_FOOTER)
    {
      finishBuffering();
      return;
    }

    final CSSNumericValue result = sectionHeight.getResult();
    if (role == OfficeDocumentReportTarget.ROLE_PAGE_HEADER)
    {
      final PageContext pageContext = getCurrentContext();
      pageContext.setHeader(applyColumnsToPageBand(finishBuffering(), pageContext.getActiveColumns()).getXmlBuffer(), result);
    }
    else if (role == OfficeDocumentReportTarget.ROLE_PAGE_FOOTER)
    {
      final PageContext pageContext = getCurrentContext();
      pageContext.setFooter(applyColumnsToPageBand(finishBuffering(), pageContext.getActiveColumns()).getXmlBuffer(), result);
    }
    else if (role == OfficeDocumentReportTarget.ROLE_REPEATING_GROUP_HEADER)
    {
      final PageContext pageContext = getCurrentContext();
      pageContext.setHeader(applyColumnsToPageBand(finishBuffering(), pageContext.getActiveColumns()).getXmlBuffer(), result);
    }
    else if (role == OfficeDocumentReportTarget.ROLE_REPEATING_GROUP_FOOTER)
    {
      final PageContext pageContext = getCurrentContext();
      pageContext.setFooter(applyColumnsToPageBand(finishBuffering(), pageContext.getActiveColumns()).getXmlBuffer(), result);
    }
    else if (role == OfficeDocumentReportTarget.ROLE_VARIABLES)
    {
      if (variables == null)
      {
        variables = finishBuffering().getXmlBuffer();
      }
      else
      {
        variables += finishBuffering().getXmlBuffer();
      }
    }
    else if (role == OfficeDocumentReportTarget.ROLE_GROUP_HEADER)
    {
      final String headerText = finishBuffering().getXmlBuffer();
      final int iterationCount = getGroupContext().getParent().getIterationCount();
      final boolean repeat = "true".equals(attrs.getAttribute(OfficeNamespaces.OOREPORT_NS, "repeat-section"));
      if (repeat == false || iterationCount > 0)
      {
        getXmlWriter().writeText(headerText);
      }
    }
    else if (role == OfficeDocumentReportTarget.ROLE_GROUP_FOOTER)
    {
      final String footerText = finishBuffering().getXmlBuffer();
      // how do we detect whether this is the last group footer?
      getXmlWriter().writeText(footerText);
    }

  }

  public void endReport(final ReportStructureRoot report)
      throws DataSourceException, ReportProcessingException
  {
    super.endReport(report);
    variablesDeclarations = null;

    try
    {
      // Write the settings ..
      final AttributeList rootAttributes = new AttributeList();
      rootAttributes.addNamespaceDeclaration("office", OfficeNamespaces.OFFICE_NS);
      rootAttributes.addNamespaceDeclaration("config", OfficeNamespaces.CONFIG);
      rootAttributes.addNamespaceDeclaration("ooo", OfficeNamespaces.OO2004_NS);
      rootAttributes.setAttribute(OfficeNamespaces.OFFICE_NS, "version", "1.0");
      final OutputStream outputStream = getOutputRepository().createOutputStream("settings.xml","text/xml");
      final XmlWriter xmlWriter = new XmlWriter(new OutputStreamWriter(outputStream, "UTF-8"), createTagDescription());
      xmlWriter.setAlwaysAddNamespace(true);
      xmlWriter.writeXmlDeclaration("UTF-8");
      xmlWriter.writeTag(OfficeNamespaces.OFFICE_NS, "document-settings", rootAttributes, XmlWriterSupport.OPEN);
      xmlWriter.writeTag(OfficeNamespaces.OFFICE_NS, "settings", XmlWriterSupport.OPEN);
      xmlWriter.writeTag(OfficeNamespaces.CONFIG, "config-item-set", "name", "ooo:configuration-settings", XmlWriterSupport.OPEN);

      final AttributeList configAttributes = new AttributeList();
      configAttributes.setAttribute(OfficeNamespaces.CONFIG, "name", "TableRowKeep");
      configAttributes.setAttribute(OfficeNamespaces.CONFIG, "type", "boolean");
      xmlWriter.writeTag(OfficeNamespaces.CONFIG, "config-item", configAttributes, XmlWriterSupport.OPEN);
      xmlWriter.writeText("true");
      xmlWriter.writeCloseTag();

      xmlWriter.writeCloseTag();
      xmlWriter.writeCloseTag();
      xmlWriter.writeCloseTag();
      xmlWriter.close();
    }
    catch (IOException ioe)
    {
      throw new ReportProcessingException("Failed to write settings document");
    }
  }

  protected void endOther(final AttributeMap attrs)
      throws IOException, DataSourceException, ReportProcessingException
  {
    if (isTableMergeActive() && detailBandProcessingState == DETAIL_SECTION_OTHER_PRINTED)
    {
      if (ReportTargetUtil.isElementOfType(OfficeNamespaces.TABLE_NS, "table-columns", attrs))
      {
        finishBuffering();
        return;
      }
    }

    if (ReportTargetUtil.isElementOfType(OfficeNamespaces.INTERNAL_NS, "image", attrs))
    {
      return;
    }

    final XmlWriter xmlWriter = getXmlWriter();
    if (tableLayoutConfig != TABLE_LAYOUT_VARIABLES_PARAGRAPH &&
        ReportTargetUtil.isElementOfType(OfficeNamespaces.TABLE_NS, "table-cell", attrs))
    {
      if (variables != null)
      {
        // This cannot happen as long as the report sections only contain tables. But at some point in the
        // future they will be made of paragraphs, and then we are prepared ..
        //Log.debug("Variables-Section " + variables);
        if (sectionKeepTogether == true && expectedTableRowCount > 0)
        {
          StyleUtilities.copyStyle("paragraph",
              TextRawReportTarget.VARIABLES_HIDDEN_STYLE_WITH_KEEPWNEXT, getStylesCollection(),
              getGlobalStylesCollection(), getPredefinedStylesCollection());
          xmlWriter.writeTag(OfficeNamespaces.TEXT_NS, "p", "style-name",
              TextRawReportTarget.VARIABLES_HIDDEN_STYLE_WITH_KEEPWNEXT, XmlWriterSupport.OPEN);
          xmlWriter.writeText(variables);
          xmlWriter.writeCloseTag();
          variables = null;
        }
        else
        {
          StyleUtilities.copyStyle("paragraph",
              TextRawReportTarget.VARIABLES_HIDDEN_STYLE_WITHOUT_KEEPWNEXT, getStylesCollection(),
              getGlobalStylesCollection(), getPredefinedStylesCollection());
          xmlWriter.writeTag(OfficeNamespaces.TEXT_NS, "p", "style-name",
              TextRawReportTarget.VARIABLES_HIDDEN_STYLE_WITHOUT_KEEPWNEXT, XmlWriterSupport.OPEN);
          xmlWriter.writeText(variables);
          xmlWriter.writeCloseTag();
          variables = null;
        }
      }
      // Only generate the empty paragraph, if we have to add the keep-together ..
      else if (cellEmpty && expectedTableRowCount > 0 &&
          sectionKeepTogether == true && firstCellSeen == false)
      {
        // we have no variables ..
        StyleUtilities.copyStyle("paragraph",
            TextRawReportTarget.VARIABLES_HIDDEN_STYLE_WITH_KEEPWNEXT, getStylesCollection(),
            getGlobalStylesCollection(), getPredefinedStylesCollection());
        xmlWriter.writeTag(OfficeNamespaces.TEXT_NS, "p", "style-name",
            TextRawReportTarget.VARIABLES_HIDDEN_STYLE_WITH_KEEPWNEXT, XmlWriterSupport.CLOSE);
      }
    }

    if (ReportTargetUtil.isElementOfType(OfficeNamespaces.TABLE_NS, "table-cell", attrs))
    {
      firstCellSeen = true;
    }
    else if (ReportTargetUtil.isElementOfType(OfficeNamespaces.TABLE_NS, "covered-table-cell", attrs))
    {
      firstCellSeen = true;
    }

    if (ReportTargetUtil.isElementOfType(OfficeNamespaces.TABLE_NS, "table", attrs))
    {
      if (getCurrentRole() == ROLE_DETAIL)
      {
        if (isTableMergeActive() == false)
        {
          // We do not merge the detail bands, so an ordinary close will do.
          xmlWriter.writeCloseTag();
        }
        else if (detailBandProcessingState == DETAIL_SECTION_FIRST_STARTED)
        {
          final int keepTogetherState = getCurrentContext().getKeepTogether();
          if (keepTogetherState == PageContext.KEEP_TOGETHER_FIRST_DETAIL)
          {
            xmlWriter.writeCloseTag();
            detailBandProcessingState = DETAIL_SECTION_FIRST_PRINTED;
          }
          else
          {
            detailBandProcessingState = DETAIL_SECTION_OTHER_PRINTED;
          }
        }
        else if (detailBandProcessingState == DETAIL_SECTION_OTHER_STARTED)
        {
          detailBandProcessingState = DETAIL_SECTION_OTHER_PRINTED;
        }
      }
      else
      {
        xmlWriter.writeCloseTag();
      }
      if (isSectionPagebreakAfter(attrs))
      {
        setPagebreakDefinition(new PageBreakDefinition(false));
      }
    }
    else
    {
      xmlWriter.writeCloseTag();
    }
  }

  protected void endGroupBody(final AttributeMap attrs)
      throws IOException, DataSourceException, ReportProcessingException
  {
    if (tableLayoutConfig == TABLE_LAYOUT_SINGLE_DETAIL_TABLE)
    {
      if (detailBandProcessingState == DETAIL_SECTION_OTHER_PRINTED)
      {
        // closes the table ..
        final XmlWriter xmlWriter = getXmlWriter();
        xmlWriter.writeCloseTag();
        detailBandProcessingState = DETAIL_SECTION_WAIT;
      }
    }

  }

  protected void endContent(final AttributeMap attrs)
      throws IOException, DataSourceException, ReportProcessingException
  {
    finishSection();
    final BufferState bodyText = finishBuffering();
    final XmlWriter writer = getXmlWriter();

    final Map definedMappings = variablesDeclarations.getDefinedMappings();
    if (definedMappings.isEmpty() == false)
    {
      writer.writeTag(OfficeNamespaces.TEXT_NS, "variable-decls", XmlWriterSupport.OPEN);
      final Iterator mappingsIt = definedMappings.entrySet().iterator();
      while (mappingsIt.hasNext())
      {
        final Map.Entry entry = (Map.Entry) mappingsIt.next();
        final AttributeList entryList = new AttributeList();
        entryList.setAttribute(OfficeNamespaces.TEXT_NS, "name", (String) entry.getKey());
        entryList.setAttribute(OfficeNamespaces.OFFICE_NS, "value-type", (String) entry.getValue());
        writer.writeTag(OfficeNamespaces.TEXT_NS, "variable-decl", entryList, XmlWriterSupport.CLOSE);
      }
      writer.writeCloseTag();
    }

    writer.writeStream(bodyText.getXmlAsReader());
    writer.setLineEmpty(true);
    writer.writeCloseTag();
  }

  public String getExportDescriptor()
  {
    return "raw/" + PentahoReportEngineMetaData.OPENDOCUMENT_TEXT;
  }

}
