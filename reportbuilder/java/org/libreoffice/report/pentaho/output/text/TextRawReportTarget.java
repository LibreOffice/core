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
package org.libreoffice.report.pentaho.output.text;


import org.libreoffice.report.DataSourceFactory;
import org.libreoffice.report.ImageService;
import org.libreoffice.report.InputRepository;
import org.libreoffice.report.OfficeToken;
import org.libreoffice.report.OutputRepository;
import org.libreoffice.report.pentaho.OfficeNamespaces;
import org.libreoffice.report.pentaho.PentahoReportEngineMetaData;
import org.libreoffice.report.pentaho.layoutprocessor.FormatValueUtility;
import org.libreoffice.report.pentaho.model.OfficeMasterPage;
import org.libreoffice.report.pentaho.model.OfficeMasterStyles;
import org.libreoffice.report.pentaho.model.OfficeStyle;
import org.libreoffice.report.pentaho.model.OfficeStyles;
import org.libreoffice.report.pentaho.model.OfficeStylesCollection;
import org.libreoffice.report.pentaho.model.PageSection;
import org.libreoffice.report.pentaho.output.OfficeDocumentReportTarget;
import org.libreoffice.report.pentaho.output.StyleUtilities;
import org.libreoffice.report.pentaho.styles.LengthCalculator;

import java.io.IOException;

import java.io.OutputStream;
import java.io.OutputStreamWriter;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.Map;


import org.jfree.layouting.input.style.values.CSSNumericValue;
import org.jfree.layouting.util.AttributeMap;
import org.jfree.report.DataSourceException;
import org.jfree.report.JFreeReportInfo;
import org.jfree.report.ReportProcessingException;
import org.jfree.report.flow.ReportJob;
import org.jfree.report.flow.ReportStructureRoot;
import org.jfree.report.flow.ReportTargetUtil;
import org.jfree.report.structure.Element;
import org.jfree.report.structure.Section;
import org.jfree.report.util.AttributeNameGenerator;
import org.jfree.report.util.IntegerCache;

import org.pentaho.reporting.libraries.base.util.FastStack;
import org.pentaho.reporting.libraries.base.util.ObjectUtilities;
import org.pentaho.reporting.libraries.resourceloader.ResourceKey;
import org.pentaho.reporting.libraries.resourceloader.ResourceManager;
import org.pentaho.reporting.libraries.xmlns.common.AttributeList;
import org.pentaho.reporting.libraries.xmlns.writer.XmlWriter;
import org.pentaho.reporting.libraries.xmlns.writer.XmlWriterSupport;


/**
 * Creation-Date: 03.07.2006, 16:28:00
 *
 */
public class TextRawReportTarget extends OfficeDocumentReportTarget
{

    private static final String ALWAYS = "always";
    private static final String KEEP_TOGETHER = "keep-together";
    private static final String KEEP_WITH_NEXT = "keep-with-next";
    private static final String MAY_BREAK_BETWEEN_ROWS = "may-break-between-rows";
    private static final String NAME = "name";
    private static final String NONE = "none";
    private static final String NORMAL = "normal";
    private static final String PARAGRAPH_PROPERTIES = "paragraph-properties";
    private static final String STANDARD = "Standard";
    private static final String TABLE_PROPERTIES = "table-properties";
    private static final String VARIABLES_HIDDEN_STYLE_WITH_KEEPWNEXT = "variables_paragraph_with_next";
    private static final String VARIABLES_HIDDEN_STYLE_WITHOUT_KEEPWNEXT = "variables_paragraph_without_next";
    private static final int TABLE_LAYOUT_VARIABLES_PARAGRAPH = 0;
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
    private final FastStack activePageContext;
    private MasterPageFactory masterPageFactory;
    private LengthCalculator sectionHeight;
    private String variables;
    private PageBreakDefinition pageBreakDefinition;
    private VariablesDeclarations variablesDeclarations;
    private boolean columnBreakPending;
    private boolean sectionKeepTogether;
    private final AttributeNameGenerator sectionNames;
    private int detailBandProcessingState;
    private final int tableLayoutConfig;
    private int expectedTableRowCount;
    private boolean firstCellSeen;

    public TextRawReportTarget(final ReportJob reportJob,
            final ResourceManager resourceManager,
            final ResourceKey baseResource,
            final InputRepository inputRepository,
            final OutputRepository outputRepository,
            final String target,
            final ImageService imageService,
            final DataSourceFactory datasourcefactory)
            throws ReportProcessingException
    {
        super(reportJob, resourceManager, baseResource, inputRepository, outputRepository, target, imageService, datasourcefactory);
        activePageContext = new FastStack();
        this.sectionNames = new AttributeNameGenerator();

        this.tableLayoutConfig = TABLE_LAYOUT_SINGLE_DETAIL_TABLE;
    }

    @Override
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
        return pageBreakDefinition != null && pageBreakDefinition.isResetPageNumber();
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
        keepWithNext = keepTogetherState == PageContext.KEEP_TOGETHER_FIRST_DETAIL && (detailBandProcessingState == DETAIL_SECTION_WAIT);
        return keepWithNext;
    }

    private boolean isSectionPagebreakAfter(final AttributeMap attrs)
    {
        final Object forceNewPage =
                attrs.getAttribute(OfficeNamespaces.OOREPORT_NS, "force-new-page");
        return "after-section".equals(forceNewPage) || "before-after-section".equals(forceNewPage);
    }

    private boolean isSectionPagebreakBefore(final AttributeMap attrs)
    {
        final Object forceNewPage =
                attrs.getAttribute(OfficeNamespaces.OOREPORT_NS, "force-new-page");
        return "before-section".equals(forceNewPage) || "before-after-section".equals(forceNewPage);
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
        // suppressed on the report-header, we have to insert a pagebreak
        // afterwards.

        final String activePageFooter;
        // Check, whether the report header can have a page-header
        final PageContext context = getCurrentContext();
        if (printFooter)
        {
            activePageFooter = context.getPageFooterContent();
        }
        else
        {
            activePageFooter = null;
        }
        final String activePageHeader;
        if (printHeader)
        {
            // we have to insert a manual pagebreak after the report header.
            activePageHeader = context.getPageHeaderContent();
        }
        else
        {
            activePageHeader = null;
        }

        final String masterPageName;
        if (currentMasterPage == null || !masterPageFactory.containsMasterPage(STANDARD, activePageHeader, activePageFooter))
        {

            final CSSNumericValue headerSize = context.getAllHeaderSize();
            final CSSNumericValue footerSize = context.getAllFooterSize();


            currentMasterPage = masterPageFactory.createMasterPage(STANDARD, activePageHeader, activePageFooter);

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
                final String derivedLayout = masterPageFactory.createPageStyle(getGlobalStylesCollection().getAutomaticStyles(), headerSize, footerSize);
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
            final OfficeMasterPage masterPage = masterPageFactory.getMasterPage(STANDARD, activePageHeader, activePageFooter);
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

        if ((!printHeader && context.getHeader() != null) || (!printFooter && context.getFooter() != null))
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
                //return null; // ignore
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
        // footer multiple times. Right now, we simply rely on the balancing, which should make sure that
        // the column's content are evenly distributed.
        final XmlWriter writer = getXmlWriter();
        final AttributeList attrs = new AttributeList();
        attrs.setAttribute(OfficeNamespaces.TEXT_NS, OfficeToken.STYLE_NAME, generateSectionStyle(numberOfColumns));
        attrs.setAttribute(OfficeNamespaces.TEXT_NS, NAME, sectionNames.generateName("Section"));
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
        sectionProperties.setAttribute(OfficeNamespaces.FO_NS, OfficeToken.BACKGROUND_COLOR, "transparent");
        sectionProperties.setAttribute(OfficeNamespaces.TEXT_NS, "dont-balance-text-columns", OfficeToken.FALSE);
        sectionProperties.setAttribute(OfficeNamespaces.STYLE_NS, "editable", OfficeToken.FALSE);

        if (columnCount > 1)
        {
            final Section columns = new Section();
            columns.setNamespace(OfficeNamespaces.STYLE_NS);
            columns.setType("columns");
            columns.setAttribute(OfficeNamespaces.FO_NS, "column-count", String.valueOf(columnCount));
            columns.setAttribute(OfficeNamespaces.STYLE_NS, "column-gap", "0cm");
            sectionProperties.addNode(columns);

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
        style.setAttribute(OfficeNamespaces.STYLE_NS, NAME, styleName);
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
     *          if some other error occurred.
     */
    @Override
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
        variablesSectionStyle.setStyleFamily(OfficeToken.PARAGRAPH);
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
        paragraphProps.setType(PARAGRAPH_PROPERTIES);
        paragraphProps.setAttribute(OfficeNamespaces.FO_NS, OfficeToken.BACKGROUND_COLOR, "transparent");
        paragraphProps.setAttribute(OfficeNamespaces.FO_NS, "text-align", "start");
        paragraphProps.setAttribute(OfficeNamespaces.FO_NS, KEEP_WITH_NEXT, ALWAYS);
        paragraphProps.setAttribute(OfficeNamespaces.FO_NS, KEEP_TOGETHER, ALWAYS);
        paragraphProps.setAttribute(OfficeNamespaces.STYLE_NS, "vertical-align", "top");
        variablesSectionStyle.addNode(paragraphProps);

        final Section textProps = new Section();
        textProps.setNamespace(OfficeNamespaces.STYLE_NS);
        textProps.setType("text-properties");
        textProps.setAttribute(OfficeNamespaces.FO_NS, "font-variant", NORMAL);
        textProps.setAttribute(OfficeNamespaces.FO_NS, "text-transform", NONE);
        textProps.setAttribute(OfficeNamespaces.FO_NS, "color", "#ffffff");
        textProps.setAttribute(OfficeNamespaces.STYLE_NS, "text-outline", OfficeToken.FALSE);
        textProps.setAttribute(OfficeNamespaces.STYLE_NS, "text-blinking", OfficeToken.FALSE);
        textProps.setAttribute(OfficeNamespaces.STYLE_NS, "text-line-through-style", NONE);
        textProps.setAttribute(OfficeNamespaces.STYLE_NS, "text-line-through-mode", "continuous");
        textProps.setAttribute(OfficeNamespaces.STYLE_NS, "text-position", "0% 100%");
        textProps.setAttribute(OfficeNamespaces.STYLE_NS, "font-name", "Tahoma");
        textProps.setAttribute(OfficeNamespaces.FO_NS, "font-size", "1pt");
        textProps.setAttribute(OfficeNamespaces.FO_NS, "letter-spacing", NORMAL);
        textProps.setAttribute(OfficeNamespaces.STYLE_NS, "letter-kerning", OfficeToken.FALSE);
        textProps.setAttribute(OfficeNamespaces.FO_NS, "font-style", NORMAL);
        textProps.setAttribute(OfficeNamespaces.FO_NS, "text-shadow", NONE);
        textProps.setAttribute(OfficeNamespaces.STYLE_NS, "text-underline-style", NONE);
        textProps.setAttribute(OfficeNamespaces.STYLE_NS, "text-underline-mode", "continuous");
        textProps.setAttribute(OfficeNamespaces.FO_NS, "font-weight", NORMAL);
        textProps.setAttribute(OfficeNamespaces.STYLE_NS, "text-rotation-angle", "0");
        textProps.setAttribute(OfficeNamespaces.STYLE_NS, "text-emphasize", NONE);
        textProps.setAttribute(OfficeNamespaces.STYLE_NS, "text-combine", NONE);
        textProps.setAttribute(OfficeNamespaces.STYLE_NS, "text-combine-start-char", "");
        textProps.setAttribute(OfficeNamespaces.STYLE_NS, "text-combine-end-char", "");
        textProps.setAttribute(OfficeNamespaces.STYLE_NS, "text-blinking", OfficeToken.FALSE);
        textProps.setAttribute(OfficeNamespaces.STYLE_NS, "text-scale", "100%");
        textProps.setAttribute(OfficeNamespaces.STYLE_NS, "font-relief", NONE);
        textProps.setAttribute(OfficeNamespaces.STYLE_NS, "text-display", NONE);
        variablesSectionStyle.addNode(textProps);
        return variablesSectionStyle;
    }

    @Override
    protected void startContent(final AttributeMap attrs)
            throws IOException, DataSourceException, ReportProcessingException
    {
        final XmlWriter xmlWriter = getXmlWriter();
        xmlWriter.writeTag(OfficeNamespaces.OFFICE_NS, "text", null, XmlWriterSupport.OPEN);

        writeNullDate();

        // now start the buffering. We have to insert the variables declaration
        // later ..
        startBuffering(getStylesCollection(), true);

        final Object columnCountRaw = attrs.getAttribute(OfficeNamespaces.FO_NS, "column-count");
        final Integer colCount = parseInt(columnCountRaw);
        if (colCount != null)
        {
            final PageContext pageContext = getCurrentContext();
            pageContext.setColumnCount(colCount);
        }

    }

    @Override
    protected void startOther(final AttributeMap attrs)
            throws IOException, DataSourceException, ReportProcessingException
    {
        final String namespace = ReportTargetUtil.getNamespaceFromAttribute(attrs);
        final String elementType = ReportTargetUtil.getElemenTypeFromAttribute(attrs);

        if (ObjectUtilities.equal(JFreeReportInfo.REPORT_NAMESPACE, namespace))
        {
            if (ObjectUtilities.equal(OfficeToken.IMAGE, elementType))
            {
                startImageProcessing(attrs);
            }
            else if (ObjectUtilities.equal(OfficeToken.OBJECT_OLE, elementType) && getCurrentRole() != ROLE_TEMPLATE)
            {
                startChartProcessing(attrs);
            }
            return;
        }
        else if (isFilteredNamespace(namespace))
        {
            throw new IllegalStateException("This element should be hidden: " + namespace + ", " + elementType);
        }

        if (isTableMergeActive() && detailBandProcessingState == DETAIL_SECTION_OTHER_PRINTED && ObjectUtilities.equal(OfficeNamespaces.TABLE_NS, namespace) && ObjectUtilities.equal(OfficeToken.TABLE_COLUMNS, elementType))
        {
            // Skip the columns section if the tables get merged..
            startBuffering(getStylesCollection(), true);
        }
        else
        {
            openSection();

            final boolean isTableNS = ObjectUtilities.equal(OfficeNamespaces.TABLE_NS, namespace);
            if (isTableNS)
            {
                if (ObjectUtilities.equal(OfficeToken.TABLE, elementType))
                {
                    startTable(attrs);
                    return;
                }

                if (ObjectUtilities.equal(OfficeToken.TABLE_ROW, elementType))
                {
                    startRow(attrs);
                    return;
                }
            }


            if (ObjectUtilities.equal(OfficeNamespaces.TEXT_NS, namespace))
            {
                if (ObjectUtilities.equal("variable-set", elementType))
                {
                    // update the variables-declaration thingie ..
                    final String varName = (String) attrs.getAttribute(OfficeNamespaces.TEXT_NS, NAME);
                    final String varType = (String) attrs.getAttribute(OfficeNamespaces.OFFICE_NS, FormatValueUtility.VALUE_TYPE);
                    final String newVarName = variablesDeclarations.produceVariable(varName, varType);
                    attrs.setAttribute(OfficeNamespaces.TEXT_NS, NAME, newVarName);
                }
                else if (ObjectUtilities.equal("variable-get", elementType))
                {
                    final String varName = (String) attrs.getAttribute(OfficeNamespaces.TEXT_NS, NAME);
                    final String varType = (String) attrs.getAttribute(OfficeNamespaces.OFFICE_NS, FormatValueUtility.VALUE_TYPE);
                    final String newVarName = variablesDeclarations.produceVariable(varName, varType);
                    attrs.setAttribute(OfficeNamespaces.TEXT_NS, NAME, newVarName);
                }
            }

            if (tableLayoutConfig == TABLE_LAYOUT_VARIABLES_PARAGRAPH && variables != null)
            {
                // This cannot happen as long as the report sections only contain tables. But at some point in the
                // future they will be made of paragraphs, and then we are prepared ..

                StyleUtilities.copyStyle(OfficeToken.PARAGRAPH,
                        TextRawReportTarget.VARIABLES_HIDDEN_STYLE_WITH_KEEPWNEXT, getStylesCollection(),
                        getGlobalStylesCollection(), getPredefinedStylesCollection());
                final XmlWriter xmlWriter = getXmlWriter();
                xmlWriter.writeTag(OfficeNamespaces.TEXT_NS, OfficeToken.P, OfficeToken.STYLE_NAME,
                        TextRawReportTarget.VARIABLES_HIDDEN_STYLE_WITH_KEEPWNEXT, XmlWriterSupport.OPEN);
                xmlWriter.writeText(variables);
                xmlWriter.writeCloseTag();
                variables = null;
            }

            final boolean keepTogetherOnParagraph = true;

            if (keepTogetherOnParagraph)
            {
                if (ReportTargetUtil.isElementOfType(OfficeNamespaces.TEXT_NS, OfficeToken.P, attrs))
                {
                    final int keepTogetherState = getCurrentContext().getKeepTogether();
                    if (!firstCellSeen && (sectionKeepTogether || keepTogetherState == PageContext.KEEP_TOGETHER_GROUP))
                    {
                        OfficeStyle style = null;
                        final String styleName = (String) attrs.getAttribute(OfficeNamespaces.TEXT_NS, OfficeToken.STYLE_NAME);
                        if (styleName == null)
                        {
                            final boolean keep = (keepTogetherState == PageContext.KEEP_TOGETHER_GROUP || expectedTableRowCount > 0) && isParentKeepTogether();
                            final ArrayList<String> propertyNameSpaces = new ArrayList<String>();
                            final ArrayList<String> propertyNames = new ArrayList<String>();
                            final ArrayList<String> propertyValues = new ArrayList<String>();

                            propertyNameSpaces.add(OfficeNamespaces.FO_NS);
                            propertyNameSpaces.add(OfficeNamespaces.FO_NS);
                            propertyNames.add(KEEP_TOGETHER);
                            propertyValues.add(ALWAYS);
                            if (keep)
                            {
                                propertyNames.add(KEEP_WITH_NEXT);
                                propertyValues.add(ALWAYS);
                            }
                            else
                            {
                                propertyNames.add(KEEP_WITH_NEXT);
                                propertyValues.add(null);
                            }
                            style = StyleUtilities.queryStyleByProperties(getStylesCollection(), OfficeToken.PARAGRAPH, PARAGRAPH_PROPERTIES, propertyNameSpaces, propertyNames, propertyValues);
                        }
                        if (style == null)
                        {
                            style = deriveStyle(OfficeToken.PARAGRAPH, styleName);
                            // Lets set the 'keep-together' flag..

                            Element paragraphProps = style.getParagraphProperties();
                            if (paragraphProps == null)
                            {
                                paragraphProps = new Section();
                                paragraphProps.setNamespace(OfficeNamespaces.STYLE_NS);
                                paragraphProps.setType(PARAGRAPH_PROPERTIES);
                                style.addNode(paragraphProps);
                            }
                            paragraphProps.setAttribute(OfficeNamespaces.FO_NS, KEEP_TOGETHER, ALWAYS);

                            // We prevent pagebreaks within the two adjacent rows (this one and the next one) if
                            // either a group-wide keep-together is defined or if we haven't reached the end of the
                            // current section yet.
                            if ((keepTogetherState == PageContext.KEEP_TOGETHER_GROUP || expectedTableRowCount > 0) && isParentKeepTogether())
                            {
                                paragraphProps.setAttribute(OfficeNamespaces.FO_NS, KEEP_WITH_NEXT, ALWAYS);
                            }
                        }

                        attrs.setAttribute(OfficeNamespaces.TEXT_NS, OfficeToken.STYLE_NAME, style.getStyleName());
                    }
                }
            }

            if (ObjectUtilities.equal(OfficeNamespaces.DRAWING_NS, namespace) && ObjectUtilities.equal(OfficeToken.FRAME, elementType))
            {
                final String styleName = (String) attrs.getAttribute(OfficeNamespaces.DRAWING_NS, OfficeToken.STYLE_NAME);
                final OfficeStyle predefAutoStyle = getPredefinedStylesCollection().getAutomaticStyles().getStyle(OfficeToken.GRAPHIC, styleName);
                if (predefAutoStyle != null)
                {
                    // special ole handling
                    final Element graphicProperties = predefAutoStyle.getGraphicProperties();
                    graphicProperties.setAttribute(OfficeNamespaces.STYLE_NS, VERTICAL_POS, "from-top");
                    graphicProperties.setAttribute(OfficeNamespaces.STYLE_NS, HORIZONTAL_POS, "from-left");
                    graphicProperties.setAttribute(OfficeNamespaces.STYLE_NS, "vertical-rel", "paragraph-content");
                    graphicProperties.setAttribute(OfficeNamespaces.STYLE_NS, "horizontal-rel", "paragraph");
                    graphicProperties.setAttribute(OfficeNamespaces.STYLE_NS, "flow-with-text", "false");
                    graphicProperties.setAttribute(OfficeNamespaces.DRAWING_NS, "ole-draw-aspect", "1");

                }
            }

            // process the styles as usual
            performStyleProcessing(attrs);
            final XmlWriter xmlWriter = getXmlWriter();
            final AttributeList attrList = buildAttributeList(attrs);
            xmlWriter.writeTag(namespace, elementType, attrList, XmlWriterSupport.OPEN);

            if (tableLayoutConfig != TABLE_LAYOUT_VARIABLES_PARAGRAPH
                    && variables != null
                    && !isRepeatingSection()
                    && ReportTargetUtil.isElementOfType(OfficeNamespaces.TEXT_NS, OfficeToken.P, attrs))
            {
                xmlWriter.writeText(variables);
                variables = null;
            }
        }
    }

    private void startRow(final AttributeMap attrs)
            throws IOException, ReportProcessingException
    {
        firstCellSeen = false;
        expectedTableRowCount -= 1;
        final String rowStyle = (String) attrs.getAttribute(OfficeNamespaces.TABLE_NS, OfficeToken.STYLE_NAME);
        final CSSNumericValue rowHeight = computeRowHeight(rowStyle);
        sectionHeight.add(rowHeight);

        // process the styles as usual
        performStyleProcessing(attrs);

        final AttributeList attrList = buildAttributeList(attrs);
        getXmlWriter().writeTag(OfficeNamespaces.TABLE_NS, OfficeToken.TABLE_ROW, attrList, XmlWriterSupport.OPEN);
    }

    private void startTable(final AttributeMap attrs)
            throws ReportProcessingException, IOException
    {
        final Integer trc = (Integer) attrs.getAttribute(JFreeReportInfo.REPORT_NAMESPACE, "table-row-count");
        if (trc == null)
        {
            expectedTableRowCount = -1;
        }
        else
        {
            expectedTableRowCount = trc;
        }

        if (isSectionPagebreakBefore(attrs))
        {
            // force a pagebreak ..
            setPagebreakDefinition(new PageBreakDefinition(isResetPageNumber()));
        }

        // it's a table. This means, it is a root-level element
        final PageBreakDefinition breakDefinition;
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
                masterPageName = createMasterPage(pageHeaderOnReportFooter, pageFooterOnReportFooter);
                if (masterPageName == null && isSectionPagebreakBefore(attrs))
                {
                    // If we have a manual pagebreak, then activate the current master-page again.
                    masterPageName = currentMasterPage.getStyleName();
                }
                // But we skip this (and therefore the resulting pagebreak) if there is no manual break
                // and no other condition that would force an break.
            }
            else if (currentRole == OfficeDocumentReportTarget.ROLE_REPEATING_GROUP_HEADER || currentRole == OfficeDocumentReportTarget.ROLE_REPEATING_GROUP_FOOTER)
            {
                breakDefinition = null;
                // no pagebreaks ..
            }
            else if (currentMasterPage == null || isPagebreakPending())
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
            else
            {
                breakDefinition = null;
            }
        }
        else if (isPagebreakPending() && currentRole != OfficeDocumentReportTarget.ROLE_REPEATING_GROUP_HEADER && currentRole != OfficeDocumentReportTarget.ROLE_REPEATING_GROUP_FOOTER)
        {
            // Derive an automatic style for the pagebreak.
            breakDefinition = getPagebreakDefinition();
            setPagebreakDefinition(null);
            masterPageName = createMasterPage(true, true);
            if (masterPageName == null || isSectionPagebreakBefore(attrs))
            {
                // If we have a manual pagebreak, then activate the current master-page again.
                masterPageName = currentMasterPage.getStyleName();
            }
        }
        else
        {
            breakDefinition = null;
        }

        final XmlWriter xmlWriter = getXmlWriter();
        if (detailBandProcessingState == DETAIL_SECTION_OTHER_PRINTED && masterPageName != null)
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

                final OfficeStyle style = deriveStyle(OfficeToken.PARAGRAPH, TextRawReportTarget.VARIABLES_HIDDEN_STYLE_WITH_KEEPWNEXT);
                style.setAttribute(OfficeNamespaces.STYLE_NS, "master-page-name", masterPageName);
                if (breakDefinition.isResetPageNumber())
                {
                    final Element paragraphProps = produceFirstChild(style, OfficeNamespaces.STYLE_NS, PARAGRAPH_PROPERTIES);
                    paragraphProps.setAttribute(OfficeNamespaces.STYLE_NS, "page-number", "1");
                }
                if (isColumnBreakPending())
                {
                    final Element paragraphProps = produceFirstChild(style, OfficeNamespaces.STYLE_NS, PARAGRAPH_PROPERTIES);
                    paragraphProps.setAttribute(OfficeNamespaces.FO_NS, "break-before", "column");
                    setColumnBreakPending(false);
                }
                xmlWriter.writeTag(OfficeNamespaces.TEXT_NS, OfficeToken.P, OfficeToken.STYLE_NAME, style.getStyleName(), XmlWriterSupport.OPEN);

                masterPageName = null;
                //breakDefinition = null;
            }
            else if (isColumnBreakPending())
            {
                setColumnBreakPending(false);

                final OfficeStyle style = deriveStyle(OfficeToken.PARAGRAPH, TextRawReportTarget.VARIABLES_HIDDEN_STYLE_WITH_KEEPWNEXT);
                final Element paragraphProps = produceFirstChild(style, OfficeNamespaces.STYLE_NS, PARAGRAPH_PROPERTIES);
                paragraphProps.setAttribute(OfficeNamespaces.STYLE_NS, "page-number", "1");

                xmlWriter.writeTag(OfficeNamespaces.TEXT_NS, OfficeToken.P, OfficeToken.STYLE_NAME, style.getStyleName(), XmlWriterSupport.OPEN);
            }
            else
            {
                // Write a paragraph without adding the pagebreak. We can reuse the global style, but we have to make
                // sure that the style is part of the current 'auto-style' collection.

                StyleUtilities.copyStyle(OfficeToken.PARAGRAPH,
                        TextRawReportTarget.VARIABLES_HIDDEN_STYLE_WITH_KEEPWNEXT, getStylesCollection(),
                        getGlobalStylesCollection(), getPredefinedStylesCollection());
                xmlWriter.writeTag(OfficeNamespaces.TEXT_NS, OfficeToken.P, OfficeToken.STYLE_NAME,
                        TextRawReportTarget.VARIABLES_HIDDEN_STYLE_WITH_KEEPWNEXT, XmlWriterSupport.OPEN);
            }
            xmlWriter.writeText(variables);
            xmlWriter.writeCloseTag();
            variables = null;
        }

        final boolean keepWithNext = isKeepTableWithNext();
        final boolean localKeepTogether = OfficeToken.TRUE.equals(attrs.getAttribute(OfficeNamespaces.OOREPORT_NS, KEEP_TOGETHER));
        final boolean tableMergeActive = isTableMergeActive();
        this.sectionKeepTogether = tableMergeActive && localKeepTogether;

        // Check, whether we have a reason to derive a style...
        if (masterPageName != null || (!tableMergeActive && (localKeepTogether || keepWithNext)) || isColumnBreakPending())
        {
            final String styleName = (String) attrs.getAttribute(OfficeNamespaces.TABLE_NS, OfficeToken.STYLE_NAME);
            final OfficeStyle style = deriveStyle("table", styleName);

            if (masterPageName != null)
            {
                // Patch the current styles.
                // This usually only happens on Table-Styles or Paragraph-Styles
                style.setAttribute(OfficeNamespaces.STYLE_NS, "master-page-name", masterPageName);
                if (breakDefinition.isResetPageNumber())
                {
                    final Element paragraphProps = produceFirstChild(style, OfficeNamespaces.STYLE_NS, PARAGRAPH_PROPERTIES);
                    paragraphProps.setAttribute(OfficeNamespaces.STYLE_NS, "page-number", "1");
                }
            }
            if (isColumnBreakPending())
            {
                final Element paragraphProps = produceFirstChild(style, OfficeNamespaces.STYLE_NS, PARAGRAPH_PROPERTIES);
                paragraphProps.setAttribute(OfficeNamespaces.FO_NS, "break-before", "column");
                setColumnBreakPending(false);
            }

            // Inhibit breaks inside the table only if it has been defined and if we do not create one single
            // big detail section. In that case, this flag would be invalid and would cause layout-errors.
            if (!tableMergeActive)
            {
                if (localKeepTogether)
                {
                    final Element tableProps = produceFirstChild(style, OfficeNamespaces.STYLE_NS, TABLE_PROPERTIES);
                    tableProps.setAttribute(OfficeNamespaces.STYLE_NS, MAY_BREAK_BETWEEN_ROWS, OfficeToken.FALSE);
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
                boolean addKeepWithNext = true;
                if (currentRole == ROLE_GROUP_FOOTER)
                {
                    addKeepWithNext = isParentKeepTogether();
                }

                final Element tableProps = produceFirstChild(style, OfficeNamespaces.STYLE_NS, TABLE_PROPERTIES);
                tableProps.setAttribute(OfficeNamespaces.STYLE_NS, MAY_BREAK_BETWEEN_ROWS, OfficeToken.FALSE);
                if (addKeepWithNext)
                {
                    tableProps.setAttribute(OfficeNamespaces.FO_NS, KEEP_WITH_NEXT, ALWAYS);
                    // A keep-with-next does not work, if the may-break-betweek rows is not set to false ..
                }
            }
            attrs.setAttribute(OfficeNamespaces.TABLE_NS, OfficeToken.STYLE_NAME, style.getStyleName());
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
                    if (keepWithNext)
                    {
                        final String styleName = (String) attrs.getAttribute(OfficeNamespaces.TABLE_NS, OfficeToken.STYLE_NAME);

                        final OfficeStyle style = deriveStyle(OfficeToken.TABLE, styleName);
                        final Element tableProps = produceFirstChild(style, OfficeNamespaces.STYLE_NS, TABLE_PROPERTIES);
                        // A keep-with-next does not work, if the may-break-betweek rows is not set to false ..
                        tableProps.setAttribute(OfficeNamespaces.STYLE_NS, MAY_BREAK_BETWEEN_ROWS, OfficeToken.FALSE);
                        final String hasGroupFooter = (String) attrs.getAttribute(JFreeReportInfo.REPORT_NAMESPACE, "has-group-footer");
                        if (hasGroupFooter != null && hasGroupFooter.equals(OfficeToken.TRUE))
                        {
                            tableProps.setAttribute(OfficeNamespaces.FO_NS, KEEP_WITH_NEXT, ALWAYS);
                        }

                        attrs.setAttribute(OfficeNamespaces.TABLE_NS, OfficeToken.STYLE_NAME, style.getStyleName());
                    }
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

    private boolean isParentKeepTogether()
    {
        PageContext context = getCurrentContext();
        if (context != null)
        {
            context = context.getParent();
            if (context != null)
            {
                return context.getKeepTogether() == PageContext.KEEP_TOGETHER_GROUP;
            }
        }
        return false;
    }

    private boolean isTableMergeActive()
    {
        return getCurrentRole() == ROLE_DETAIL && tableLayoutConfig == TABLE_LAYOUT_SINGLE_DETAIL_TABLE;
    }

    private void openSection()
            throws IOException
    {
        if (isRepeatingSection())
        {
            // repeating sections have other ways of defining columns ..
            return;
        }
        if (getCurrentRole() == ROLE_TEMPLATE || getCurrentRole() == ROLE_SPREADSHEET_PAGE_HEADER || getCurrentRole() == ROLE_SPREADSHEET_PAGE_FOOTER)
        {
            // the template section would break the multi-column stuff and we don't open up sections there
            // anyway ..
            return;
        }

        final PageContext pageContext = getCurrentContext();
        final Integer columnCount = pageContext.getColumnCount();
        if (columnCount != null && !pageContext.isSectionOpen())
        {
            final AttributeList attrs = new AttributeList();
            attrs.setAttribute(OfficeNamespaces.TEXT_NS, OfficeToken.STYLE_NAME, generateSectionStyle(columnCount));
            attrs.setAttribute(OfficeNamespaces.TEXT_NS, NAME, sectionNames.generateName("Section"));
            getXmlWriter().writeTag(OfficeNamespaces.TEXT_NS, "section", attrs, XmlWriterSupport.OPEN);

            pageContext.setSectionOpen(true);
        }

    }

    @Override
    protected void startReportSection(final AttributeMap attrs, final int role)
            throws ReportProcessingException
    {
        sectionHeight = new LengthCalculator();
        if (role == OfficeDocumentReportTarget.ROLE_TEMPLATE || role == OfficeDocumentReportTarget.ROLE_SPREADSHEET_PAGE_HEADER || role == OfficeDocumentReportTarget.ROLE_SPREADSHEET_PAGE_FOOTER)
        {
            // Start buffering with an dummy styles-collection, so that the global styles don't get polluted ..
            startBuffering(new OfficeStylesCollection(), true);
        }
        else if (role == OfficeDocumentReportTarget.ROLE_PAGE_HEADER)
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
        else if (role == OfficeDocumentReportTarget.ROLE_REPEATING_GROUP_HEADER || role == OfficeDocumentReportTarget.ROLE_REPEATING_GROUP_FOOTER)
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
            if (role == OfficeDocumentReportTarget.ROLE_GROUP_HEADER || role == OfficeDocumentReportTarget.ROLE_GROUP_FOOTER)
            {
                // if we have a repeating header, then skip the first one ..
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

    @Override
    protected void startGroup(final AttributeMap attrs)
    {
        super.startGroup(attrs);
        final PageContext pageContext = new PageContext(getCurrentContext());
        activePageContext.push(pageContext);

        final Object resetPageNumber = attrs.getAttribute(OfficeNamespaces.OOREPORT_NS, "reset-page-number");
        if (OfficeToken.TRUE.equals(resetPageNumber))
        {
            setPagebreakDefinition(new PageBreakDefinition(true));
        }

        final Object keepTogether = attrs.getAttribute(OfficeNamespaces.OOREPORT_NS, KEEP_TOGETHER);
        if ("whole-group".equals(keepTogether))
        {
            pageContext.setKeepTogether(PageContext.KEEP_TOGETHER_GROUP);
        }
        else if ("with-first-detail".equals(keepTogether) && pageContext.getKeepTogether() != PageContext.KEEP_TOGETHER_GROUP)
        {
            pageContext.setKeepTogether(PageContext.KEEP_TOGETHER_FIRST_DETAIL);
        }

        final Object columnCountRaw = attrs.getAttribute(OfficeNamespaces.FO_NS, "column-count");
        final Integer colCount = parseInt(columnCountRaw);
        if (colCount != null)
        {
            pageContext.setColumnCount(colCount);
        }

        final Object newColumn = attrs.getAttribute(OfficeNamespaces.OOREPORT_NS, "start-new-column");
        if (OfficeToken.TRUE.equals(newColumn))
        {
            setColumnBreakPending(true);
        }
    }

    @Override
    protected void startGroupInstance(final AttributeMap attrs)
    {
        if (getGroupContext().isGroupWithRepeatingSection())
        {
            setPagebreakDefinition(new PageBreakDefinition(isResetPageNumber()));
        }
    }

    @Override
    protected void endGroup(final AttributeMap attrs)
            throws ReportProcessingException
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

    @Override
    protected void endReportSection(final AttributeMap attrs, final int role)
            throws IOException, ReportProcessingException
    {
        if (role == ROLE_TEMPLATE || role == OfficeDocumentReportTarget.ROLE_SPREADSHEET_PAGE_HEADER || role == OfficeDocumentReportTarget.ROLE_SPREADSHEET_PAGE_FOOTER)
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
            final boolean repeat = OfficeToken.TRUE.equals(attrs.getAttribute(OfficeNamespaces.OOREPORT_NS, "repeat-section"));
            if (!repeat || iterationCount > 0)
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

    @Override
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
            rootAttributes.setAttribute(OfficeNamespaces.OFFICE_NS, "version",
                    OfficeDocumentReportTarget.ODF_VERSION);
            final OutputStream outputStream = getOutputRepository().createOutputStream("settings.xml", "text/xml");
            final XmlWriter xmlWriter = new XmlWriter(new OutputStreamWriter(outputStream, "UTF-8"), createTagDescription());
            xmlWriter.setAlwaysAddNamespace(true);
            xmlWriter.writeXmlDeclaration("UTF-8");
            xmlWriter.writeTag(OfficeNamespaces.OFFICE_NS, "document-settings", rootAttributes, XmlWriterSupport.OPEN);
            xmlWriter.writeTag(OfficeNamespaces.OFFICE_NS, "settings", XmlWriterSupport.OPEN);
            xmlWriter.writeTag(OfficeNamespaces.CONFIG, "config-item-set", NAME, "ooo:configuration-settings", XmlWriterSupport.OPEN);

            final AttributeList configAttributes = new AttributeList();
            configAttributes.setAttribute(OfficeNamespaces.CONFIG, NAME, "TableRowKeep");
            configAttributes.setAttribute(OfficeNamespaces.CONFIG, "type", "boolean");
            xmlWriter.writeTag(OfficeNamespaces.CONFIG, "config-item", configAttributes, XmlWriterSupport.OPEN);
            xmlWriter.writeText(OfficeToken.TRUE);
            xmlWriter.writeCloseTag();

            xmlWriter.writeCloseTag();
            xmlWriter.writeCloseTag();
            xmlWriter.writeCloseTag();
            xmlWriter.close();

            copyMeta();
        }
        catch (IOException ioe)
        {
            throw new ReportProcessingException("Failed to write settings document", ioe);
        }
    }

    @Override
    protected void endOther(final AttributeMap attrs)
            throws IOException, DataSourceException, ReportProcessingException
    {
        final String namespace = ReportTargetUtil.getNamespaceFromAttribute(attrs);
        final String elementType = ReportTargetUtil.getElemenTypeFromAttribute(attrs);

        final boolean isInternalNS = ObjectUtilities.equal(JFreeReportInfo.REPORT_NAMESPACE, namespace);
        final boolean isTableNs = ObjectUtilities.equal(OfficeNamespaces.TABLE_NS, namespace);
        if (isTableMergeActive() && detailBandProcessingState == DETAIL_SECTION_OTHER_PRINTED && isTableNs && ObjectUtilities.equal(OfficeToken.TABLE_COLUMNS, elementType))
        {
            finishBuffering();
            return;
        }

        if (isInternalNS && (ObjectUtilities.equal(OfficeToken.IMAGE, elementType) || ObjectUtilities.equal(OfficeToken.OBJECT_OLE, elementType)))
        {
            return;
        }

        final XmlWriter xmlWriter = getXmlWriter();
        if (tableLayoutConfig != TABLE_LAYOUT_VARIABLES_PARAGRAPH && isTableNs && ObjectUtilities.equal(OfficeToken.TABLE_CELL, elementType) && !isRepeatingSection())
        {
            if (variables != null)
            {
                // This cannot happen as long as the report sections only contain tables. But at some point in the
                // future they will be made of paragraphs, and then we are prepared ..
                final String tag;
                if (sectionKeepTogether && expectedTableRowCount > 0)
                {
                    tag = TextRawReportTarget.VARIABLES_HIDDEN_STYLE_WITH_KEEPWNEXT;
                }
                else
                {
                    tag = TextRawReportTarget.VARIABLES_HIDDEN_STYLE_WITHOUT_KEEPWNEXT;
                }
                StyleUtilities.copyStyle(OfficeToken.PARAGRAPH,
                        tag, getStylesCollection(),
                        getGlobalStylesCollection(), getPredefinedStylesCollection());
                xmlWriter.writeTag(OfficeNamespaces.TEXT_NS, OfficeToken.P, OfficeToken.STYLE_NAME,
                        tag, XmlWriterSupport.OPEN);
                xmlWriter.writeText(variables);
                xmlWriter.writeCloseTag();
                variables = null;
            }
            /**
            // Only generate the empty paragraph, if we have to add the keep-together ..
            else if (cellEmpty && expectedTableRowCount > 0 &&
            sectionKeepTogether && !firstCellSeen)
            {
            // we have no variables ..
            StyleUtilities.copyStyle(OfficeToken.PARAGRAPH,
            TextRawReportTarget.VARIABLES_HIDDEN_STYLE_WITH_KEEPWNEXT, getStylesCollection(),
            getGlobalStylesCollection(), getPredefinedStylesCollection());
            xmlWriter.writeTag(OfficeNamespaces.TEXT_NS, OfficeToken.P, OfficeToken.STYLE_NAME,
            TextRawReportTarget.VARIABLES_HIDDEN_STYLE_WITH_KEEPWNEXT, XmlWriterSupport.CLOSE);
            }
             */
        }

        if (isTableNs && (ObjectUtilities.equal(OfficeToken.TABLE_CELL, elementType) || ObjectUtilities.equal(OfficeToken.COVERED_TABLE_CELL, elementType)))
        {
            firstCellSeen = true;
        }
        if (isTableNs && ObjectUtilities.equal(OfficeToken.TABLE, elementType))
        {
            if (getCurrentRole() == ROLE_DETAIL)
            {
                if (!isTableMergeActive())
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

    @Override
    protected void endGroupBody(final AttributeMap attrs)
            throws IOException
    {
        if (tableLayoutConfig == TABLE_LAYOUT_SINGLE_DETAIL_TABLE && detailBandProcessingState == DETAIL_SECTION_OTHER_PRINTED)
        {
            // closes the table ..
            final XmlWriter xmlWriter = getXmlWriter();
            xmlWriter.writeCloseTag();
            detailBandProcessingState = DETAIL_SECTION_WAIT;
        }

    }

    @Override
    protected void endContent(final AttributeMap attrs)
            throws IOException, DataSourceException, ReportProcessingException
    {
        finishSection();
        final BufferState bodyText = finishBuffering();
        final XmlWriter writer = getXmlWriter();

        final Map definedMappings = variablesDeclarations.getDefinedMappings();
        if (!definedMappings.isEmpty())
        {
            writer.writeTag(OfficeNamespaces.TEXT_NS, "variable-decls", XmlWriterSupport.OPEN);
            final Iterator mappingsIt = definedMappings.entrySet().iterator();
            while (mappingsIt.hasNext())
            {
                final Map.Entry entry = (Map.Entry) mappingsIt.next();
                final AttributeList entryList = new AttributeList();
                entryList.setAttribute(OfficeNamespaces.TEXT_NS, NAME, (String) entry.getKey());
                entryList.setAttribute(OfficeNamespaces.OFFICE_NS, FormatValueUtility.VALUE_TYPE, (String) entry.getValue());
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
