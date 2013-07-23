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
package org.libreoffice.report.pentaho.output;

import org.libreoffice.report.DataSourceFactory;
import org.libreoffice.report.ImageService;
import org.libreoffice.report.InputRepository;
import org.libreoffice.report.OfficeToken;
import org.libreoffice.report.OutputRepository;
import org.libreoffice.report.ReportEngineParameterNames;
import org.libreoffice.report.SDBCReportDataFactory;
import org.libreoffice.report.pentaho.OfficeNamespaces;
import org.libreoffice.report.pentaho.layoutprocessor.ImageElementContext;
import org.libreoffice.report.pentaho.model.OfficeDocument;
import org.libreoffice.report.pentaho.model.OfficeStyle;
import org.libreoffice.report.pentaho.model.OfficeStyles;
import org.libreoffice.report.pentaho.model.OfficeStylesCollection;
import org.libreoffice.report.pentaho.styles.LengthCalculator;
import org.libreoffice.report.pentaho.styles.StyleMapper;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.DocumentBuilder;
import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.InputSource;

import java.awt.Image;

import java.io.BufferedReader;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.Reader;
import java.io.StringReader;
import java.io.StringWriter;
import java.io.Writer;
import java.io.InputStream;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.Map;
import java.util.zip.DeflaterOutputStream;
import java.util.zip.InflaterInputStream;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import org.jfree.layouting.input.style.parser.CSSValueFactory;
import org.jfree.layouting.input.style.parser.StyleSheetParserUtil;
import org.jfree.layouting.input.style.values.CSSNumericType;
import org.jfree.layouting.input.style.values.CSSNumericValue;
import org.jfree.layouting.layouter.style.CSSValueResolverUtility;
import org.jfree.layouting.namespace.NamespaceDefinition;
import org.jfree.layouting.namespace.Namespaces;
import org.jfree.layouting.util.AttributeMap;
import org.jfree.layouting.util.LazyAttributeMap;
import org.jfree.report.DataFlags;
import org.jfree.report.DataSourceException;
import org.jfree.report.JFreeReportBoot;
import org.jfree.report.JFreeReportInfo;
import org.jfree.report.ReportProcessingException;
import org.jfree.report.flow.AbstractReportTarget;
import org.jfree.report.flow.ReportJob;
import org.jfree.report.flow.ReportStructureRoot;
import org.jfree.report.flow.ReportTargetUtil;
import org.jfree.report.structure.Element;
import org.jfree.report.structure.Section;
import org.jfree.report.util.AttributeNameGenerator;
import org.jfree.report.util.IntegerCache;
import org.jfree.report.util.MemoryByteArrayOutputStream;

import org.pentaho.reporting.libraries.base.util.FastStack;
import org.pentaho.reporting.libraries.base.util.IOUtils;
import org.pentaho.reporting.libraries.resourceloader.ResourceException;
import org.pentaho.reporting.libraries.resourceloader.ResourceKey;
import org.pentaho.reporting.libraries.resourceloader.ResourceManager;
import org.pentaho.reporting.libraries.xmlns.common.AttributeList;
import org.pentaho.reporting.libraries.xmlns.writer.DefaultTagDescription;
import org.pentaho.reporting.libraries.xmlns.writer.XmlWriter;
import org.pentaho.reporting.libraries.xmlns.writer.XmlWriterSupport;

import org.w3c.css.sac.LexicalUnit;

/**
 * Todo: Document me!
 *
 * @since 08.03.2007
 */
public abstract class OfficeDocumentReportTarget extends AbstractReportTarget
{

    protected static final Log LOGGER = LogFactory.getLog(OfficeDocumentReportTarget.class);
    public static final String HORIZONTAL_POS = "horizontal-pos";
    public static final String TAG_DEF_PREFIX = "org.libreoffice.report.pentaho.output.";
    public static final int ROLE_NONE = 0;
    public static final int ROLE_REPORT_HEADER = 1;
    public static final int ROLE_REPORT_FOOTER = 2;
    public static final int ROLE_GROUP_HEADER = 3;
    public static final int ROLE_GROUP_FOOTER = 4;
    public static final int ROLE_REPEATING_GROUP_HEADER = 5;
    public static final int ROLE_REPEATING_GROUP_FOOTER = 6;
    public static final int ROLE_PAGE_HEADER = 7;
    public static final int ROLE_PAGE_FOOTER = 8;
    public static final int ROLE_DETAIL = 9;
    public static final int ROLE_VARIABLES = 10;
    public static final int ROLE_TEMPLATE = 11;
    public static final int ROLE_SPREADSHEET_PAGE_HEADER = 12;
    public static final int ROLE_SPREADSHEET_PAGE_FOOTER = 13;
    public static final int STATE_IN_DOCUMENT = 0;
    public static final int STATE_IN_BODY = 1;
    public static final int STATE_IN_CONTENT = 2;
    public static final int STATE_IN_GROUP = 3;
    public static final int STATE_IN_GROUP_BODY = 4;
    public static final int STATE_IN_SECTION = 5;
    public static final int STATE_IN_OTHER = 6;
    public static final int STATE_IN_GROUP_INSTANCE = 7;
    public static final String FAILED = "Failed";
    public static final String VERTICAL_POS = "vertical-pos";
    private static final String ZERO_CM = "0cm";
    /** the verison of the ODF specification to which generated documents
     *  shall conform. */
    public static final String ODF_VERSION = "1.2";

    protected static class BufferState
    {

        private final XmlWriter xmlWriter;
        private final MemoryByteArrayOutputStream xmlBuffer;
        private final OfficeStylesCollection stylesCollection;

        protected BufferState(final XmlWriter xmlWriter,
                final MemoryByteArrayOutputStream xmlBuffer,
                final OfficeStylesCollection stylesCollection)
        {
            this.stylesCollection = stylesCollection;
            this.xmlWriter = xmlWriter;
            this.xmlBuffer = xmlBuffer;
        }

        public OfficeStylesCollection getStylesCollection()
        {
            return stylesCollection;
        }

        public XmlWriter getXmlWriter()
        {
            return xmlWriter;
        }

        public String getXmlBuffer() throws ReportProcessingException
        {
            try
            {
                final byte[] zippedData = xmlBuffer.getRaw();
                final InputStreamReader reader = new InputStreamReader(new InflaterInputStream(new ByteArrayInputStream(zippedData, 0, xmlBuffer.getLength())), "UTF-16");
                final StringWriter writer = new StringWriter((zippedData.length / 2) + 1);
                IOUtils.getInstance().copyWriter(reader, writer);
                return writer.toString();
            }
            catch (IOException e)
            {
                throw new ReportProcessingException("Failed to copy buffer", e);
            }
        }

        public Reader getXmlAsReader() throws ReportProcessingException
        {
            try
            {
                final byte[] zippedData = xmlBuffer.getRaw();
                return new InputStreamReader(new InflaterInputStream(new ByteArrayInputStream(zippedData, 0, xmlBuffer.getLength())), "UTF-16");
            }
            catch (IOException e)
            {
                throw new ReportProcessingException("Failed to copy buffer", e);
            }
        }
    }

    protected static class GroupContext
    {

        private final GroupContext parent;
        private int iterationCount;
        private boolean groupWithRepeatingSection;

        protected GroupContext(final GroupContext parent)
        {
            this.parent = parent;
        }

        public GroupContext getParent()
        {
            return parent;
        }

        public int getIterationCount()
        {
            return iterationCount;
        }

        public void setIterationCount(final int iterationCount)
        {
            this.iterationCount = iterationCount;
        }

        public boolean isGroupWithRepeatingSection()
        {
            return groupWithRepeatingSection;
        }

        public void setGroupWithRepeatingSection(final boolean groupWithRepeatingSection)
        {
            this.groupWithRepeatingSection = groupWithRepeatingSection;
        }

        public String toString()
        {
            return "GroupContext{" + "parent=" + parent + ", iterationCount=" + iterationCount + ", groupWithRepeatingSection=" + groupWithRepeatingSection + '}';
        }
    }
    private final FastStack states;
    private int currentRole;
    private final FastStack xmlWriters;
    private XmlWriter rootXmlWriter;
    /**
     * This styles-collection contains all styles that were predefined in the report definition file. The common styles
     * and the master-styles will be written unmodified, the automatic styles will be ignored.
     */
    private OfficeStylesCollection predefinedStylesCollection;
    /**
     * This styles-collection contains all master-styles that have been generated by the report definition process. It
     * also contains all automatic styles that have been generated for the page-bands (and the pagebands as well).
     */
    private OfficeStylesCollection globalStylesCollection;
    /**
     * The content styles collection contains all automatic styles that have been generated for the normal-flow content.
     */
    private OfficeStylesCollection contentStylesCollection;
    private final OutputRepository outputRepository;
    private final InputRepository inputRepository;
    private final AttributeNameGenerator tableNameGenerator;
    private final AttributeNameGenerator frameNameGenerator;
    private final AttributeNameGenerator autoStyleNameGenerator;
    private final String target;
    private static final int INITIAL_BUFFER_SIZE = 40960;
    private StyleMapper styleMapper;
    private StyleSheetParserUtil styleSheetParserUtil;
    private final AttributeNameGenerator imageNames;
    private final ImageProducer imageProducer;
    private final OleProducer oleProducer;
    private GroupContext groupContext;
    private static final boolean DEBUG_ELEMENTS =
            JFreeReportBoot.getInstance().getExtendedConfig().getBoolProperty("org.libreoffice.report.pentaho.output.DebugElements");

    protected OfficeDocumentReportTarget(final ReportJob reportJob,
            final ResourceManager resourceManager,
            final ResourceKey baseResource,
            final InputRepository inputRepository,
            final OutputRepository outputRepository,
            final String target,
            final ImageService imageService,
            final DataSourceFactory datasourcefactory)
            throws ReportProcessingException
    {
        super(reportJob, resourceManager, baseResource);
        if (imageService == null)
        {
            throw new NullPointerException("ImageService must not be null");
        }
        if (target == null)
        {
            throw new NullPointerException("Target-Name must not be null");
        }

        this.target = target;

        this.tableNameGenerator = new AttributeNameGenerator();
        this.frameNameGenerator = new AttributeNameGenerator();
        this.autoStyleNameGenerator = new AttributeNameGenerator();
        this.outputRepository = outputRepository;
        this.inputRepository = inputRepository;
        this.states = new FastStack();
        this.xmlWriters = new FastStack();
        this.imageNames = new AttributeNameGenerator();

        this.imageProducer = new ImageProducer(inputRepository, outputRepository, imageService);
        this.oleProducer = new OleProducer(inputRepository, outputRepository, imageService, datasourcefactory, (Integer) reportJob.getParameters().get(ReportEngineParameterNames.MAXROWS));

        try
        {
            final ResourceManager realResourceManager = getResourceManager();
            styleMapper = StyleMapper.loadInstance(realResourceManager);
        }
        catch (ResourceException e)
        {
            throw new ReportProcessingException("Failed to load style-mapper", e);
        }
    }

    protected abstract String getTargetMimeType();

    protected OutputRepository getOutputRepository()
    {
        return outputRepository;
    }

    protected InputRepository getInputRepository()
    {
        return inputRepository;
    }

    /**
     * Starts the output of a new office document. This method writes the generic 'office:document-content' tag along with
     * all known namespace declarations.
     *
     * @param report the report object.
     * @throws DataSourceException       if there was an error accessing the datasource
     * @throws ReportProcessingException if some other error occured.
     */
    public void startReport(final ReportStructureRoot report)
            throws DataSourceException, ReportProcessingException
    {
        imageNames.reset();
        this.groupContext = new GroupContext(null);

        final DefaultTagDescription tagDescription = createTagDescription();
        try
        {
            final OutputStream outputStream = outputRepository.createOutputStream(target, "text/xml");
            final Writer writer = new OutputStreamWriter(outputStream, "UTF-8");

            this.rootXmlWriter = new XmlWriter(writer, tagDescription);
            this.rootXmlWriter.setAlwaysAddNamespace(true);

            final AttributeList rootAttributes = new AttributeList();
            rootAttributes.addNamespaceDeclaration("office", OfficeNamespaces.OFFICE_NS);
            rootAttributes.addNamespaceDeclaration("style", OfficeNamespaces.STYLE_NS);
            rootAttributes.addNamespaceDeclaration("text", OfficeNamespaces.TEXT_NS);
            rootAttributes.addNamespaceDeclaration("table", OfficeNamespaces.TABLE_NS);
            rootAttributes.addNamespaceDeclaration("draw", OfficeNamespaces.DRAWING_NS);
            rootAttributes.addNamespaceDeclaration("fo", OfficeNamespaces.FO_NS);
            rootAttributes.addNamespaceDeclaration("xlink", OfficeNamespaces.XLINK_NS);
            rootAttributes.addNamespaceDeclaration("dc", OfficeNamespaces.PURL_NS);
            rootAttributes.addNamespaceDeclaration("meta", OfficeNamespaces.META_NS);
            rootAttributes.addNamespaceDeclaration("number", OfficeNamespaces.DATASTYLE_NS);
            rootAttributes.addNamespaceDeclaration("svg", OfficeNamespaces.SVG_NS);
            rootAttributes.addNamespaceDeclaration("chart", OfficeNamespaces.CHART_NS);
            rootAttributes.addNamespaceDeclaration("chartooo", OfficeNamespaces.CHARTOOO_NS);
            rootAttributes.addNamespaceDeclaration("dr3d", OfficeNamespaces.DR3D_NS);
            rootAttributes.addNamespaceDeclaration("math", OfficeNamespaces.MATHML_NS);
            rootAttributes.addNamespaceDeclaration("form", OfficeNamespaces.FORM_NS);
            rootAttributes.addNamespaceDeclaration("script", OfficeNamespaces.SCRIPT_NS);
            rootAttributes.addNamespaceDeclaration("ooo", OfficeNamespaces.OO2004_NS);
            rootAttributes.addNamespaceDeclaration("ooow", OfficeNamespaces.OOW2004_NS);
            rootAttributes.addNamespaceDeclaration("oooc", OfficeNamespaces.OOC2004_NS);
            rootAttributes.addNamespaceDeclaration("dom", OfficeNamespaces.XML_EVENT_NS);
            rootAttributes.addNamespaceDeclaration("xforms", OfficeNamespaces.XFORMS_NS);
            rootAttributes.addNamespaceDeclaration("xsd", OfficeNamespaces.XSD_NS);
            rootAttributes.addNamespaceDeclaration("xsi", OfficeNamespaces.XSI_NS);
            rootAttributes.addNamespaceDeclaration("grddl", OfficeNamespaces.GRDDL_NS);
            rootAttributes.setAttribute(OfficeNamespaces.OFFICE_NS, "version",
                    ODF_VERSION);

            this.rootXmlWriter.writeXmlDeclaration("UTF-8");
            this.rootXmlWriter.writeTag(OfficeNamespaces.OFFICE_NS, "document-content", rootAttributes, XmlWriterSupport.OPEN);

            states.push(IntegerCache.getInteger(OfficeDocumentReportTarget.STATE_IN_DOCUMENT));

            autoStyleNameGenerator.reset();
            tableNameGenerator.reset();
            frameNameGenerator.reset();

            final OfficeDocument reportDoc = (OfficeDocument) report;
            predefinedStylesCollection = reportDoc.getStylesCollection();

            final OfficeStyles commonStyles = predefinedStylesCollection.getCommonStyles();
            if (!commonStyles.containsStyle(OfficeToken.GRAPHIC, OfficeToken.GRAPHICS))
            {
                final OfficeStyle graphicsDefaultStyle = new OfficeStyle();
                graphicsDefaultStyle.setStyleFamily(OfficeToken.GRAPHIC);
                graphicsDefaultStyle.setStyleName(OfficeToken.GRAPHICS);
                final Element graphicProperties = produceFirstChild(graphicsDefaultStyle, OfficeNamespaces.STYLE_NS, OfficeToken.GRAPHIC_PROPERTIES);
                graphicProperties.setAttribute(OfficeNamespaces.TEXT_NS, "anchor-type", OfficeToken.PARAGRAPH);
                graphicProperties.setAttribute(OfficeNamespaces.SVG_NS, "x", ZERO_CM);
                graphicProperties.setAttribute(OfficeNamespaces.SVG_NS, "y", ZERO_CM);
                graphicProperties.setAttribute(OfficeNamespaces.STYLE_NS, "wrap", "dynamic");
                graphicProperties.setAttribute(OfficeNamespaces.STYLE_NS, "number-wrapped-paragraphs", "no-limit");
                graphicProperties.setAttribute(OfficeNamespaces.STYLE_NS, "wrap-contour", OfficeToken.FALSE);
                graphicProperties.setAttribute(OfficeNamespaces.STYLE_NS, VERTICAL_POS, "from-top"); // changed for chart

                graphicProperties.setAttribute(OfficeNamespaces.STYLE_NS, "vertical-rel", OfficeToken.PARAGRAPH);
                graphicProperties.setAttribute(OfficeNamespaces.STYLE_NS, HORIZONTAL_POS, "from-left"); // changed for chart

                graphicProperties.setAttribute(OfficeNamespaces.STYLE_NS, "horizontal-rel", OfficeToken.PARAGRAPH);
                commonStyles.addStyle(graphicsDefaultStyle);
            }

            // Make sure that later generated styles do not overwrite existing styles.
            fillStyleNameGenerator(predefinedStylesCollection);

            contentStylesCollection = new OfficeStylesCollection();
            globalStylesCollection = new OfficeStylesCollection();

            startBuffering(contentStylesCollection, true);
        }
        catch (IOException e)
        {
            throw new ReportProcessingException(FAILED, e);
        }
    }

    protected AttributeNameGenerator getAutoStyleNameGenerator()
    {
        return autoStyleNameGenerator;
    }

    private void fillStyleNameGenerator(final OfficeStylesCollection stylesCollection)
    {
        final OfficeStyles commonStyles = stylesCollection.getCommonStyles();
        final OfficeStyle[] allCommonStyles = commonStyles.getAllStyles();
        for (int i = 0; i < allCommonStyles.length; i++)
        {
            final OfficeStyle style = allCommonStyles[i];
            autoStyleNameGenerator.generateName(style.getStyleName());
        }

        final OfficeStyles autoStyles = stylesCollection.getAutomaticStyles();
        final OfficeStyle[] allAutoStyles = autoStyles.getAllStyles();
        for (int i = 0; i < allAutoStyles.length; i++)
        {
            final OfficeStyle style = allAutoStyles[i];
            autoStyleNameGenerator.generateName(style.getStyleName());
        }
    }

    public OfficeStylesCollection getPredefinedStylesCollection()
    {
        return predefinedStylesCollection;
    }

    public OfficeStylesCollection getGlobalStylesCollection()
    {
        return globalStylesCollection;
    }

    public OfficeStylesCollection getContentStylesCollection()
    {
        return contentStylesCollection;
    }

    /**
     * Returns the XML-Writer tag description. This description defines whether an element can have character data inside.
     * Such element will disable the indention, as in that case the additional whitespaces might alter the meaning of the
     * element's contents.
     *
     * @return the tag description library.
     */
    protected DefaultTagDescription createTagDescription()
    {
        final DefaultTagDescription tagDescription = new DefaultTagDescription();
        tagDescription.configure(JFreeReportBoot.getInstance().getGlobalConfig(),
                OfficeDocumentReportTarget.TAG_DEF_PREFIX);
        return tagDescription;
    }

    /**
     * Returns the current processing state.
     *
     * @return the processing state.
     */
    protected int getCurrentState()
    {
        if (states.isEmpty())
        {
            throw new IllegalStateException();
        }
        final Integer o = (Integer) states.peek();
        return o;
    }

    /**
     * Starts the processing of an element and updates the processing state. This will select an apropriate handler method
     * for the call and will call one of the start* methods.
     *
     * @param roAttrs the attribute map for the current element
     * @throws DataSourceException
     * @throws ReportProcessingException
     */
    public final void startElement(final AttributeMap roAttrs)
            throws DataSourceException, ReportProcessingException
    {
        final AttributeMap attrs = new LazyAttributeMap(roAttrs);
        // todo
        if (DEBUG_ELEMENTS)
        {
            LOGGER.debug("Starting " + getCurrentState() + '/' + states.size() + ' ' + ReportTargetUtil.getNamespaceFromAttribute(attrs) + " -> " + ReportTargetUtil.getElemenTypeFromAttribute(attrs));
        }
        try
        {
            switch (getCurrentState())
            {
                case OfficeDocumentReportTarget.STATE_IN_DOCUMENT:
                {
                    if (ReportTargetUtil.isElementOfType(OfficeNamespaces.OFFICE_NS, "body", attrs))
                    {
                        states.push(IntegerCache.getInteger(OfficeDocumentReportTarget.STATE_IN_BODY));
                        startBody(attrs);
                    }
                    else
                    {
                        states.push(IntegerCache.getInteger(OfficeDocumentReportTarget.STATE_IN_OTHER));
                        if (!isFilteredNamespace(ReportTargetUtil.getNamespaceFromAttribute(attrs)))
                        {
                            startOther(attrs);
                        }
                    }
                    break;
                }
                case OfficeDocumentReportTarget.STATE_IN_BODY:
                {
                    if (ReportTargetUtil.isElementOfType(OfficeNamespaces.OFFICE_NS, "report", attrs))
                    {
                        states.push(IntegerCache.getInteger(OfficeDocumentReportTarget.STATE_IN_CONTENT));
                        startContent(attrs);
                    }
                    else
                    {
                        throw new IllegalStateException("The 'office:body' element must have exactly one child of type 'report'");
                    }
                    break;
                }
                case OfficeDocumentReportTarget.STATE_IN_CONTENT:
                {
                    // Either a ordinary section or a group ..
                    // A group.
                    if (ReportTargetUtil.isElementOfType(JFreeReportInfo.REPORT_NAMESPACE, "report-body", attrs))
                    {
                        states.push(IntegerCache.getInteger(OfficeDocumentReportTarget.STATE_IN_GROUP_BODY));
                        startGroupBody(attrs);
                    }
                    else
                    {
                        // Either a template-section, page-header, page-footer, report-header, report-footer
                        // or variables-section
                        states.push(IntegerCache.getInteger(OfficeDocumentReportTarget.STATE_IN_SECTION));
                        if (ReportTargetUtil.isElementOfType(JFreeReportInfo.REPORT_NAMESPACE, "template", attrs))
                        {
                            currentRole = OfficeDocumentReportTarget.ROLE_TEMPLATE;
                        }
                        else if (ReportTargetUtil.isElementOfType(OfficeNamespaces.OOREPORT_NS, "page-header", attrs))
                        {
                            if ("spreadsheet-section".equals(attrs.getAttribute(JFreeReportInfo.REPORT_NAMESPACE, "role")))
                            {
                                currentRole = OfficeDocumentReportTarget.ROLE_SPREADSHEET_PAGE_HEADER;
                            }
                            else
                            {
                                currentRole = OfficeDocumentReportTarget.ROLE_PAGE_HEADER;
                            }
                        }
                        else if (ReportTargetUtil.isElementOfType(OfficeNamespaces.OOREPORT_NS, "page-footer", attrs))
                        {
                            if ("spreadsheet-section".equals(attrs.getAttribute(JFreeReportInfo.REPORT_NAMESPACE, "role")))
                            {
                                currentRole = OfficeDocumentReportTarget.ROLE_SPREADSHEET_PAGE_FOOTER;
                            }
                            else
                            {
                                currentRole = OfficeDocumentReportTarget.ROLE_PAGE_FOOTER;
                            }
                        }
                        else if (ReportTargetUtil.isElementOfType(OfficeNamespaces.OOREPORT_NS, "report-header", attrs))
                        {
                            currentRole = OfficeDocumentReportTarget.ROLE_REPORT_HEADER;
                        }
                        else if (ReportTargetUtil.isElementOfType(OfficeNamespaces.OOREPORT_NS, "report-footer", attrs))
                        {
                            currentRole = OfficeDocumentReportTarget.ROLE_REPORT_FOOTER;
                        }
                        else if (ReportTargetUtil.isElementOfType(JFreeReportInfo.REPORT_NAMESPACE, "variables-section", attrs))
                        {
                            currentRole = OfficeDocumentReportTarget.ROLE_VARIABLES;
                        }
                        else
                        {
                            throw new IllegalStateException("Expected either 'template', 'report-body', " + "'report-header', 'report-footer', 'variables-section', 'page-header' or 'page-footer'");
                        }
                        startReportSection(attrs, currentRole);
                    }
                    break;
                }
                case OfficeDocumentReportTarget.STATE_IN_GROUP_BODY:
                {
                    // We now expect either an other group or a detail band.

                    if (ReportTargetUtil.isElementOfType(OfficeNamespaces.OOREPORT_NS, "group", attrs))
                    {
                        states.push(IntegerCache.getInteger(OfficeDocumentReportTarget.STATE_IN_GROUP));
                        groupContext = new GroupContext(groupContext);
                        startGroup(attrs);
                    }
                    else
                    {
                        // Either a variables-section, or a detail-band
                        states.push(IntegerCache.getInteger(OfficeDocumentReportTarget.STATE_IN_SECTION));
                        if (ReportTargetUtil.isElementOfType(OfficeNamespaces.OOREPORT_NS, "detail", attrs))
                        {
                            currentRole = OfficeDocumentReportTarget.ROLE_DETAIL;
                        }
                        else if (ReportTargetUtil.isElementOfType(JFreeReportInfo.REPORT_NAMESPACE, "variables-section", attrs))
                        {
                            currentRole = OfficeDocumentReportTarget.ROLE_VARIABLES;
                        }
                        else
                        {
                            throw new IllegalStateException("Expected either 'group', 'detail' or 'variables-section'");
                        }
                        startReportSection(attrs, currentRole);
                    }
                    break;
                }
                case OfficeDocumentReportTarget.STATE_IN_GROUP:
                {
                    // A group can carry a repeating group header/footer or a group-instance section.
                    if (ReportTargetUtil.isElementOfType(JFreeReportInfo.REPORT_NAMESPACE, "group-instance", attrs))
                    {
                        states.push(IntegerCache.getInteger(OfficeDocumentReportTarget.STATE_IN_GROUP_INSTANCE));
                        startGroupInstance(attrs);
                    }
                    else
                    {
                        // repeating group header/footer, but *no* variables section
                        states.push(IntegerCache.getInteger(OfficeDocumentReportTarget.STATE_IN_SECTION));
                        if (ReportTargetUtil.isElementOfType(OfficeNamespaces.OOREPORT_NS, "group-header", attrs) && OfficeToken.TRUE.equals(attrs.getAttribute(JFreeReportInfo.REPORT_NAMESPACE, "repeated-section")))
                        {
                            currentRole = OfficeDocumentReportTarget.ROLE_REPEATING_GROUP_HEADER;
                        }
                        else if (ReportTargetUtil.isElementOfType(OfficeNamespaces.OOREPORT_NS, "group-footer", attrs) && OfficeToken.TRUE.equals(attrs.getAttribute(JFreeReportInfo.REPORT_NAMESPACE, "repeated-section")))
                        {
                            currentRole = OfficeDocumentReportTarget.ROLE_REPEATING_GROUP_FOOTER;
                        }
                        else
                        {
                            throw new IllegalStateException("Expected either 'group-instance', " + "'repeating group-header' or 'repeating group-footer'");
                        }
                        startReportSection(attrs, currentRole);
                    }
                    break;
                }
                case OfficeDocumentReportTarget.STATE_IN_GROUP_INSTANCE:
                {
                    if (ReportTargetUtil.isElementOfType(JFreeReportInfo.REPORT_NAMESPACE, "group-body", attrs))
                    {
                        states.push(IntegerCache.getInteger(OfficeDocumentReportTarget.STATE_IN_GROUP_BODY));
                        startGroupBody(attrs);
                    }
                    else
                    {
                        // Either a group-header or group-footer or variables-section
                        states.push(IntegerCache.getInteger(OfficeDocumentReportTarget.STATE_IN_SECTION));
                        if (ReportTargetUtil.isElementOfType(OfficeNamespaces.OOREPORT_NS, "group-header", attrs))
                        {
                            currentRole = OfficeDocumentReportTarget.ROLE_GROUP_HEADER;
                        }
                        else if (ReportTargetUtil.isElementOfType(OfficeNamespaces.OOREPORT_NS, "group-footer", attrs))
                        {
                            currentRole = OfficeDocumentReportTarget.ROLE_GROUP_FOOTER;
                        }
                        else if (ReportTargetUtil.isElementOfType(JFreeReportInfo.REPORT_NAMESPACE, "variables-section", attrs))
                        {
                            currentRole = OfficeDocumentReportTarget.ROLE_VARIABLES;
                        }
                        else
                        {
                            throw new IllegalStateException("Expected either 'group-body', 'group-header', 'group-footer' or 'variables-section'");
                        }
                        startReportSection(attrs, currentRole);
                    }
                    break;
                }
                case OfficeDocumentReportTarget.STATE_IN_SECTION:
                {
                    states.push(IntegerCache.getInteger(OfficeDocumentReportTarget.STATE_IN_OTHER));
                    startOther(attrs);
                    break;
                }
                case OfficeDocumentReportTarget.STATE_IN_OTHER:
                {
                    states.push(IntegerCache.getInteger(OfficeDocumentReportTarget.STATE_IN_OTHER));
                    startOther(attrs);
                    break;
                }
                default:
                    throw new IllegalStateException("Failure: " + getCurrentState());
            }
        }
        catch (IOException ioe)
        {
            LOGGER.error("ReportProcessing failed", ioe);
            throw new ReportProcessingException("Failed to write content", ioe);
        }
//    finally
//    {
//      LOGGER.debug ("Started " + getNamespaceFromAttribute(attrs) + ":" +
//                 getElemenTypeFromAttribute(attrs) + " -> " + getCurrentState());
//    }
    }

    protected GroupContext getGroupContext()
    {
        return groupContext;
    }

    protected void performStyleProcessing(final AttributeMap attrs)
            throws ReportProcessingException
    {
        final OfficeStylesCollection stylesCollection = getStylesCollection();
        final OfficeStylesCollection predefCollection = getPredefinedStylesCollection();
        final OfficeStylesCollection globalStylesCollection = getGlobalStylesCollection();

        final String elementNamespace =
                ReportTargetUtil.getNamespaceFromAttribute(attrs);
        final String elementName =
                ReportTargetUtil.getElemenTypeFromAttribute(attrs);

        final String[] namespaces = attrs.getNameSpaces();
        for (int i = 0; i < namespaces.length; i++)
        {
            final String attrNamespace = namespaces[i];
            if (isFilteredNamespace(attrNamespace))
            {
                continue;
            }

            final Map attributes = attrs.getAttributes(attrNamespace);
            final Iterator iterator = attributes.entrySet().iterator();
            while (iterator.hasNext())
            {
                final Map.Entry entry = (Map.Entry) iterator.next();
                final String attrName = (String) entry.getKey();
                final String attrValue = String.valueOf(entry.getValue());

                final String styleFamily = styleMapper.getStyleFamilyFor(elementNamespace, elementName, attrNamespace, attrName);
                if (styleFamily == null)
                {
                    // None of the known style attributes.
                    continue;
                }

                if (styleMapper.isListOfStyles(elementNamespace, elementName, attrNamespace, attrName))
                {
                    // ignored for now.
                    LOGGER.warn("List of styles is not yet implemented.");
                    continue;
                }

                // Copy styles is only called once per style.
                StyleUtilities.copyStyle(styleFamily, attrValue, stylesCollection, globalStylesCollection, predefCollection);
            }
        }
    }

    protected void startBody(final AttributeMap attrs)
            throws IOException
    {
        getXmlWriter().writeTag(OfficeNamespaces.OFFICE_NS, "body", XmlWriterSupport.OPEN);
    }

    private final boolean allowBuffering(final int role)
    {
        return (role == OfficeDocumentReportTarget.ROLE_REPEATING_GROUP_FOOTER || role == OfficeDocumentReportTarget.ROLE_REPEATING_GROUP_HEADER || role == OfficeDocumentReportTarget.ROLE_TEMPLATE);
    }

    protected void startReportSection(final AttributeMap attrs, final int role)
            throws IOException, DataSourceException, ReportProcessingException
    {
        if (allowBuffering(role))
        {
            startBuffering(new OfficeStylesCollection(), true);
        }
    }

    protected abstract void startContent(final AttributeMap attrs)
            throws IOException, DataSourceException, ReportProcessingException;

    protected void startGroup(final AttributeMap attrs)
            throws IOException, DataSourceException, ReportProcessingException
    {
        final Object repeatingHeaderOrFooter = attrs.getAttribute(JFreeReportInfo.REPORT_NAMESPACE, "repeating-header-or-footer");
        if (OfficeToken.TRUE.equals(repeatingHeaderOrFooter))
        {
            getGroupContext().setGroupWithRepeatingSection(true);
        }

        final Object iterationCount = attrs.getAttribute(JFreeReportInfo.REPORT_NAMESPACE, "iteration-count");
        if (iterationCount instanceof Number)
        {
            final Number itNumber = (Number) iterationCount;
            getGroupContext().setIterationCount(itNumber.intValue());
        }
    }

    protected void startGroupInstance(final AttributeMap attrs)
            throws IOException, DataSourceException, ReportProcessingException
    {
    }

    protected void startGroupBody(final AttributeMap attrs)
            throws IOException, DataSourceException, ReportProcessingException
    {
    }

    protected abstract void startOther(final AttributeMap attrs)
            throws IOException, DataSourceException, ReportProcessingException;

    public void processText(final String text)
            throws DataSourceException, ReportProcessingException
    {
        try
        {
            final XmlWriter xmlWriter = getXmlWriter();
            final BufferedReader br = new BufferedReader(new StringReader(text));
            String line = br.readLine();
            while (line != null)
            {
                xmlWriter.writeTextNormalized(line, false);
                line = br.readLine();
                if (line != null)
                {
                    xmlWriter.writeTag(OfficeNamespaces.TEXT_NS, "line-break", XmlWriterSupport.CLOSE);
                }
            }
        }
        catch (IOException e)
        {
            throw new ReportProcessingException(FAILED, e);
        }
    }

    protected boolean isFilteredNamespace(final String namespace)
    {
        if (Namespaces.LIBLAYOUT_NAMESPACE.equals(namespace))
        {
            return true;
        }
        if (JFreeReportInfo.REPORT_NAMESPACE.equals(namespace))
        {
            return true;
        }
        if (JFreeReportInfo.REPORT_NAMESPACE.equals(namespace))
        {
            return true;
        }
        if (JFreeReportInfo.COMPATIBILITY_NAMESPACE.equals(namespace))
        {
            return true;
        }
        if (OfficeNamespaces.OOREPORT_NS.equals(namespace))
        {
            return true;
        }
        return false;
    }

    public void processContent(final DataFlags value)
            throws DataSourceException, ReportProcessingException
    {
        final Object rawvalue = value.getValue();
        if (rawvalue == null)
        {
            return;
        }

        // special handler for image (possibly also for URL ..)
        if (rawvalue instanceof Image)
        {
            // do nothing yet. We should define something for that later ..
            return;
        }

        final XmlWriter xmlWriter = getXmlWriter();
        final String text = String.valueOf(rawvalue);
        try
        {
            final BufferedReader br = new BufferedReader(new StringReader(text));
            String line = br.readLine();
            while (line != null)
            {
                xmlWriter.writeTextNormalized(line, false);
                line = br.readLine();
                if (line != null)
                {
                    xmlWriter.writeTag(OfficeNamespaces.TEXT_NS, "line-break", XmlWriterSupport.CLOSE);
                }
            }
        }
        catch (IOException e)
        {
            throw new ReportProcessingException(FAILED, e);
        }
    }

    public final void endElement(final AttributeMap roAttrs)
            throws DataSourceException, ReportProcessingException
    {
        final AttributeMap attrs = new LazyAttributeMap(roAttrs);
        // final int oldState = getCurrentState();
        try
        {

            switch (getCurrentState())
            {
                case OfficeDocumentReportTarget.STATE_IN_OTHER:
                {
                    endOther(attrs);
                    break;
                }
                case OfficeDocumentReportTarget.STATE_IN_SECTION:
                {
                    endReportSection(attrs, currentRole);
                    currentRole = OfficeDocumentReportTarget.ROLE_NONE;
                    break;
                }
                case OfficeDocumentReportTarget.STATE_IN_GROUP:
                {
                    endGroup(attrs);
                    groupContext = groupContext.getParent();
                    break;
                }
                case OfficeDocumentReportTarget.STATE_IN_GROUP_INSTANCE:
                {
                    endGroupInstance(attrs);
                    break;
                }
                case OfficeDocumentReportTarget.STATE_IN_GROUP_BODY:
                {
                    endGroupBody(attrs);
                    break;
                }
                case OfficeDocumentReportTarget.STATE_IN_CONTENT:
                {
                    endContent(attrs);
                    break;
                }
                case OfficeDocumentReportTarget.STATE_IN_BODY:
                {
                    endBody(attrs);
                    break;
                }
                case OfficeDocumentReportTarget.STATE_IN_DOCUMENT:
                {
                    throw new IllegalStateException("This cannot be.");
                }
                default:
                {
                    throw new IllegalStateException("Invalid state encountered.");
                }
            }
        }
        catch (IOException ioe)
        {
            throw new ReportProcessingException("IO Error while writing content",
                    ioe);
        } finally
        {
            states.pop();

            if (DEBUG_ELEMENTS)
            {
                LOGGER.debug("Finished " + getCurrentState() + "/" + states.size() + " " + ReportTargetUtil.getNamespaceFromAttribute(attrs) + ":" + ReportTargetUtil.getElemenTypeFromAttribute(attrs));
            }

        }
    }

    protected void endGroupBody(final AttributeMap attrs)
            throws IOException, DataSourceException, ReportProcessingException
    {
    }

    protected void endGroupInstance(final AttributeMap attrs)
            throws IOException, DataSourceException, ReportProcessingException
    {
    }

    public int getCurrentRole()
    {
        return currentRole;
    }

    protected abstract void endOther(final AttributeMap attrs)
            throws IOException, DataSourceException, ReportProcessingException;

    protected void endReportSection(final AttributeMap attrs,
            final int role)
            throws IOException, DataSourceException, ReportProcessingException
    {
        if (allowBuffering(role))
        {
            finishBuffering();
        }
    }

    protected void endGroup(final AttributeMap attrs)
            throws IOException, DataSourceException, ReportProcessingException
    {
    }

    protected abstract void endContent(final AttributeMap attrs)
            throws IOException, DataSourceException, ReportProcessingException;

    protected void endBody(final AttributeMap attrs)
            throws IOException, DataSourceException, ReportProcessingException
    {
        getXmlWriter().writeCloseTag();
    }

    public void copyMeta()
    {
        // now copy the meta.xml
        if (getInputRepository().isReadable("meta.xml"))
        {
            InputStream inputStream = null;
            try
            {
                inputStream = getInputRepository().createInputStream("meta.xml");
                DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
                DocumentBuilder dBuilder = dbFactory.newDocumentBuilder();
                Document document = dBuilder.parse(new InputSource(inputStream));

                NodeList nl = document.getElementsByTagName("document-meta/meta/generator");
                Node node = document.getFirstChild().getFirstChild().getFirstChild().getFirstChild();
                String creator = node.getNodeValue();
                node.setNodeValue(creator + "/report_builder");
                Transformer transformer = TransformerFactory.newInstance().newTransformer();
                transformer.setOutputProperty(OutputKeys.METHOD, "xml");

                final OutputStream outputMetaStream = getOutputRepository().createOutputStream("meta.xml", "text/xml");
                StreamResult result = new StreamResult(outputMetaStream);
                DOMSource source = new DOMSource(document);
                transformer.transform(source, result);

                //IOUtils.getInstance().copyStreams(inputStream, outputMetaStream);
                outputMetaStream.flush();
                outputMetaStream.close();
            }
            catch (java.lang.Exception ex)
            {
            } finally
            {
                if (inputStream != null)
                {
                    try
                    {
                        inputStream.close();
                    }
                    catch (IOException ex)
                    {
                        Logger.getLogger(OfficeDocumentReportTarget.class.getName()).log(Level.SEVERE, null, ex);
                    }
                }
            }
        }
    }

    public void endReport(final ReportStructureRoot report)
            throws DataSourceException, ReportProcessingException
    {
        if (xmlWriters.size() != 1)
        {
            throw new IllegalStateException("Invalid writer-stack state");
        }

        try
        {
            final StylesWriter inlineStylesWriter = new StylesWriter(rootXmlWriter);
            inlineStylesWriter.writeContentStyles(predefinedStylesCollection, contentStylesCollection);

            final BufferState state = finishBuffering();
            this.rootXmlWriter.writeStream(state.getXmlAsReader());

            final OutputStream stylesOutStream =
                    outputRepository.createOutputStream("styles.xml", "text/xml");
            final OutputStreamWriter osw =
                    new OutputStreamWriter(stylesOutStream, "UTF-8");
            final StylesWriter stylesWriter = new StylesWriter(osw);
            stylesWriter.writeGlobalStyles(predefinedStylesCollection, globalStylesCollection);
            stylesWriter.close();

            this.rootXmlWriter.writeCloseTag();
            this.rootXmlWriter.close();
        }
        catch (IOException e)
        {
            throw new ReportProcessingException(FAILED, e);
        }
    }

    public XmlWriter getXmlWriter()
    {
        final BufferState bufferState = (BufferState) xmlWriters.peek();
        return bufferState.getXmlWriter();
    }

    public OfficeStylesCollection getStylesCollection()
    {
        final BufferState bufferState = (BufferState) xmlWriters.peek();
        return bufferState.getStylesCollection();
    }

    public void startBuffering(final OfficeStylesCollection stylesCollection,
            final boolean indent) throws ReportProcessingException
    {
        final XmlWriter currentWriter;
        if (xmlWriters.isEmpty())
        {
            currentWriter = rootXmlWriter;
        }
        else
        {
            final BufferState bufferState = (BufferState) xmlWriters.peek();
            currentWriter = bufferState.getXmlWriter();
        }

        try
        {
            final MemoryByteArrayOutputStream out =
                    new MemoryByteArrayOutputStream(INITIAL_BUFFER_SIZE, 256 * INITIAL_BUFFER_SIZE);
            final DeflaterOutputStream deflateOut = new DeflaterOutputStream(out);
            final OutputStreamWriter xmlBuffer = new OutputStreamWriter(deflateOut, "UTF-16");
            //    final StringWriter xmlBuffer = new StringWriter
            //        (OfficeDocumentReportTarget.INITIAL_BUFFER_SIZE);
            final XmlWriter contentXmlWriter = new XmlWriter(xmlBuffer, createTagDescription());
            contentXmlWriter.copyNamespaces(currentWriter);
            if (indent)
            {
                contentXmlWriter.setAdditionalIndent(currentWriter.getCurrentIndentLevel());
                contentXmlWriter.setWriteFinalLinebreak(true);
            }
            else
            {
                contentXmlWriter.setWriteFinalLinebreak(false);
            }
            contentXmlWriter.setAlwaysAddNamespace(true);
            xmlWriters.push(new BufferState(contentXmlWriter, out, stylesCollection));
        }
        catch (IOException ioe)
        {
            throw new ReportProcessingException("Unable to create the buffer", ioe);
        }
    }

    public BufferState finishBuffering() throws ReportProcessingException
    {
        final BufferState state = (BufferState) xmlWriters.pop();
        try
        {
            state.getXmlWriter().close();
        }
        catch (IOException e)
        {
            LOGGER.error("ReportProcessing failed", e);
        }
        return state;
    }

    public void commit()
            throws ReportProcessingException
    {
        // do not call flush before the report is fully finished. Every flush
        // causes the Office-Backend to fully ZIP all contents (it acts like a
        // 'Save' call from the UI) and that's expensive like hell
    }

    public NamespaceDefinition getNamespaceByUri(final String uri)
    {
        return null;
    }

    protected AttributeList buildAttributeList(final AttributeMap attrs)
    {
        final String elementType = ReportTargetUtil.getElemenTypeFromAttribute(attrs);
        final AttributeList attrList = new AttributeList();
        final String[] namespaces = attrs.getNameSpaces();
        for (int i = 0; i < namespaces.length; i++)
        {
            final String attrNamespace = namespaces[i];
            if (isFilteredNamespace(attrNamespace))
            {
                continue;
            }

            final Map localAttributes = attrs.getAttributes(attrNamespace);
            final Iterator entries = localAttributes.entrySet().iterator();
            while (entries.hasNext())
            {
                final Map.Entry entry = (Map.Entry) entries.next();
                final String key = String.valueOf(entry.getKey());
                if (OfficeNamespaces.TABLE_NS.equals(attrNamespace) && "name".equals(key))
                {
                    final String tableName = String.valueOf(entry.getValue());
                    final String saneName = sanitizeName(tableName);
                    attrList.setAttribute(attrNamespace, key,
                            tableNameGenerator.generateName(saneName));
                }
                else if (OfficeNamespaces.DRAWING_NS.equals(attrNamespace) && "name".equals(key) && !"equation".equals(elementType))
                {
                    final String objectName = String.valueOf(entry.getValue());
                    attrList.setAttribute(attrNamespace, key,
                            frameNameGenerator.generateName(objectName));
                }
                else
                {
                    attrList.setAttribute(attrNamespace, key, String.valueOf(entry.getValue()));
                }
            }
        }
        return attrList;
    }

    protected String sanitizeName(final String name)
    {
        // A table name cannot contain spaces and should only contain
        // ascii-characters.
        if (name == null)
        {
            return "";
        }
        final char[] chars = name.toCharArray();
        final StringBuffer buffer = new StringBuffer();
        for (int i = 0; i < chars.length; i++)
        {
            final char aChar = chars[i];
            if (Character.isWhitespace(aChar))
            {
                buffer.append('_');
            }
            else
            {
                buffer.append(aChar);
            }
        }
        return buffer.toString();
    }

    /**
     * Returns the length in point. This method is f**king slow, it eats half of the processing time. I surely should
     * replace it with something more efficient later.
     *
     * @param text
     * @return
     */
    protected CSSNumericValue parseLength(final String text)
    {
        if (styleSheetParserUtil == null)
        {
            styleSheetParserUtil = StyleSheetParserUtil.getInstance();
        }

        final LexicalUnit cssValue = styleSheetParserUtil.parseLexicalStyleValue(
                text);
        return CSSValueFactory.createLengthValue(cssValue);
    }

    protected boolean isRepeatingSection()
    {
        return (currentRole == OfficeDocumentReportTarget.ROLE_REPEATING_GROUP_FOOTER || currentRole == OfficeDocumentReportTarget.ROLE_REPEATING_GROUP_HEADER || currentRole == OfficeDocumentReportTarget.ROLE_PAGE_FOOTER || currentRole == OfficeDocumentReportTarget.ROLE_PAGE_HEADER || currentRole == OfficeDocumentReportTarget.ROLE_VARIABLES);

    }

    protected OfficeStyle deriveStyle(final String styleFamily, final String styleName)
            throws ReportProcessingException
    {
        // autogenerate a style. The style has already been added to the current
        // auto-collection.
        final OfficeStyle style = StyleUtilities.deriveStyle(styleFamily, styleName,
                getStylesCollection(), getGlobalStylesCollection(),
                getPredefinedStylesCollection(), getAutoStyleNameGenerator());
        return style;
    }

    protected void startImageProcessing(final AttributeMap attrs)
            throws ReportProcessingException
    {
        final Object imageData = attrs.getAttribute(JFreeReportInfo.REPORT_NAMESPACE, OfficeToken.IMAGE_DATA);
        final boolean preserveIRI = OfficeToken.TRUE.equals(attrs.getAttribute(JFreeReportInfo.REPORT_NAMESPACE, OfficeToken.PRESERVE_IRI));

        // for the first shot, do nothing fancy ..
        final ImageProducer.OfficeImage image = imageProducer.produceImage(imageData, preserveIRI);
        if (image != null)
        {
            final ImageElementContext imageContext = (ImageElementContext) attrs.getAttribute(JFreeReportInfo.REPORT_NAMESPACE, "image-context");

            // When scaling, we have to create an image-style.
            final CSSNumericValue width = image.getWidth(); // always in 100th of a mm

            final CSSNumericValue height = image.getHeight(); // always in 100th of a mm

            LOGGER.debug("Image " + imageData + " Width: " + width + ", Height: " + height);
            if (width == null || height == null)
            {
                return;
            }

            CSSNumericValue imageAreaWidthVal;
            CSSNumericValue imageAreaHeightVal;
            CSSNumericValue posX = CSSNumericValue.createValue(CSSNumericType.CM, 0.0);
            CSSNumericValue posY = CSSNumericValue.createValue(CSSNumericType.CM, 0.0);

            String styleName = null;
            if (imageContext != null)
            {
                imageAreaWidthVal = computeImageWidth(imageContext);
                imageAreaHeightVal = computeImageHeight(imageContext);

                if (imageAreaWidthVal == null || imageAreaHeightVal == null)
                {
                    LOGGER.debug("Image data returned from context is invalid. Maybe this is not an image?");
                    return;
                }
                else
                {
                    // compute the clip-area ..
                    final CSSNumericValue normalizedImageWidth =
                            CSSValueResolverUtility.convertLength(width, imageAreaWidthVal.getType());
                    final CSSNumericValue normalizedImageHeight =
                            CSSValueResolverUtility.convertLength(height, imageAreaHeightVal.getType());

                    final String scale = (String) attrs.getAttribute(JFreeReportInfo.REPORT_NAMESPACE, OfficeToken.SCALE);
                    if (OfficeToken.NONE.equals(scale) && normalizedImageWidth.getValue() > 0 && normalizedImageHeight.getValue() > 0)
                    {
                        final double clipWidth = normalizedImageWidth.getValue() - imageAreaWidthVal.getValue();
                        final double clipHeight = normalizedImageHeight.getValue() - imageAreaHeightVal.getValue();
                        if (clipWidth > 0 && clipHeight > 0)
                        {
                            final OfficeStyle imageStyle = deriveStyle(OfficeToken.GRAPHIC, OfficeToken.GRAPHICS);
                            final Element graphProperties = produceFirstChild(imageStyle, OfficeNamespaces.STYLE_NS, OfficeToken.GRAPHIC_PROPERTIES);
                            final StringBuffer buffer = new StringBuffer();
                            buffer.append("rect(");
                            buffer.append(clipHeight / 2);
                            buffer.append(imageAreaHeightVal.getType().getType());
                            buffer.append(' ');
                            buffer.append(clipWidth / 2);
                            buffer.append(imageAreaWidthVal.getType().getType());
                            buffer.append(' ');
                            buffer.append(clipHeight / 2);
                            buffer.append(imageAreaHeightVal.getType().getType());
                            buffer.append(' ');
                            buffer.append(clipWidth / 2);
                            buffer.append(imageAreaWidthVal.getType().getType());
                            buffer.append(')');
                            graphProperties.setAttribute(OfficeNamespaces.FO_NS, "clip", buffer.toString());

                            styleName = imageStyle.getStyleName();
                            getStylesCollection().getAutomaticStyles().addStyle(imageStyle);
                        }
                        else if (clipWidth > 0)
                        {
                            final OfficeStyle imageStyle = deriveStyle(OfficeToken.GRAPHIC, OfficeToken.GRAPHICS);
                            final Element graphProperties = produceFirstChild(imageStyle, OfficeNamespaces.STYLE_NS, OfficeToken.GRAPHIC_PROPERTIES);
                            final StringBuffer buffer = new StringBuffer();
                            buffer.append("rect(0cm ");
                            buffer.append(clipWidth / 2);
                            buffer.append(imageAreaWidthVal.getType().getType());
                            buffer.append(" 0cm ");
                            buffer.append(clipWidth / 2);
                            buffer.append(imageAreaWidthVal.getType().getType());
                            buffer.append(')');
                            graphProperties.setAttribute(OfficeNamespaces.FO_NS, "clip", buffer.toString());

                            styleName = imageStyle.getStyleName();
                            getStylesCollection().getAutomaticStyles().addStyle(imageStyle);
                            imageAreaHeightVal = normalizedImageHeight;
                        }
                        else if (clipHeight > 0)
                        {
                            final OfficeStyle imageStyle = deriveStyle(OfficeToken.GRAPHIC, OfficeToken.GRAPHICS);
                            final Element graphProperties = produceFirstChild(imageStyle, OfficeNamespaces.STYLE_NS, OfficeToken.GRAPHIC_PROPERTIES);
                            final StringBuffer buffer = new StringBuffer();
                            buffer.append("rect(");
                            buffer.append(clipHeight / 2);
                            buffer.append(imageAreaHeightVal.getType().getType());
                            buffer.append(" 0cm ");
                            buffer.append(clipHeight / 2);
                            buffer.append(imageAreaHeightVal.getType().getType());
                            buffer.append(" 0cm)");
                            graphProperties.setAttribute(OfficeNamespaces.FO_NS, "clip", buffer.toString());

                            styleName = imageStyle.getStyleName();
                            getStylesCollection().getAutomaticStyles().addStyle(imageStyle);
                            imageAreaWidthVal = normalizedImageWidth;
                        }
                        else
                        {
                            imageAreaWidthVal = normalizedImageWidth;
                            imageAreaHeightVal = normalizedImageHeight;
                        }
                    }
                    else if (OfficeToken.ISOTROPIC.equals(scale))
                    {
                        final double[] ret = calcPaintSize(imageAreaWidthVal, imageAreaHeightVal, normalizedImageWidth, normalizedImageHeight);

                        posX = CSSNumericValue.createValue(imageAreaWidthVal.getType(), (imageAreaWidthVal.getValue() - ret[0]) * 0.5);
                        posY = CSSNumericValue.createValue(imageAreaHeightVal.getType(), (imageAreaHeightVal.getValue() - ret[1]) * 0.5);

                        imageAreaWidthVal = CSSNumericValue.createValue(imageAreaWidthVal.getType(), ret[0]);
                        imageAreaHeightVal = CSSNumericValue.createValue(imageAreaHeightVal.getType(), ret[1]);
                    }
                }
                // If we do scale, then we simply use the given image-area-size as valid image size and dont
                // care about the image itself ..
            }
            else
            {
                LOGGER.debug("There is no image-context, so we have to rely on the image's natural bounds. " + "This may go awfully wrong.");
                imageAreaWidthVal = image.getWidth();
                imageAreaHeightVal = image.getHeight();
            }

            final AttributeList frameList = new AttributeList();
            frameList.setAttribute(OfficeNamespaces.DRAWING_NS, "name", imageNames.generateName("Image"));
            if (styleName != null)
            {
                frameList.setAttribute(OfficeNamespaces.DRAWING_NS, OfficeToken.STYLE_NAME, styleName);
            }
            frameList.setAttribute(OfficeNamespaces.TEXT_NS, "anchor-type", OfficeToken.PARAGRAPH);
            frameList.setAttribute(OfficeNamespaces.SVG_NS, "z-index", "0");
            frameList.setAttribute(OfficeNamespaces.SVG_NS, "x", posX.getValue() + posX.getType().getType());
            frameList.setAttribute(OfficeNamespaces.SVG_NS, "y", posY.getValue() + posY.getType().getType());


            LOGGER.debug("Image " + imageData + " A-Width: " + imageAreaWidthVal + ", A-Height: " + imageAreaHeightVal);

            if (imageAreaWidthVal != null)
            {
                frameList.setAttribute(OfficeNamespaces.SVG_NS,
                        "width", imageAreaWidthVal.getValue() + imageAreaWidthVal.getType().getType());
            }

            if (imageAreaHeightVal != null)
            {
                frameList.setAttribute(OfficeNamespaces.SVG_NS,
                        "height", imageAreaHeightVal.getValue() + imageAreaHeightVal.getType().getType());
            }


            final AttributeList imageList = new AttributeList();
            imageList.setAttribute(OfficeNamespaces.XLINK_NS, "href", image.getEmbeddableLink());
            imageList.setAttribute(OfficeNamespaces.XLINK_NS, "type", "simple");
            imageList.setAttribute(OfficeNamespaces.XLINK_NS, "show", "embed");
            imageList.setAttribute(OfficeNamespaces.XLINK_NS, "actuate", "onLoad");


            try
            {
                getXmlWriter().writeTag(OfficeNamespaces.DRAWING_NS, "frame", frameList, XmlWriterSupport.OPEN);
                getXmlWriter().writeTag(OfficeNamespaces.DRAWING_NS, OfficeToken.IMAGE, imageList, XmlWriterSupport.CLOSE);
                getXmlWriter().writeCloseTag();
            }
            catch (IOException ioe)
            {
                throw new ReportProcessingException(FAILED, ioe);
            }
        }
    }

    private CSSNumericValue computeImageWidth(final ImageElementContext imageElementContext)
    {
        final LengthCalculator calculator = new LengthCalculator();
        final String[] strings = imageElementContext.getColStyles();
        for (int i = 0; i < strings.length; i++)
        {
            final String styleName = strings[i];
            final CSSNumericValue value = computeColumnWidth(styleName);
            if (value != null)
            {
                calculator.add(value);
            }
        }
        return calculator.getResult();
    }

    private CSSNumericValue computeImageHeight(final ImageElementContext imageElementContext)
    {
        final LengthCalculator calculator = new LengthCalculator();
        final String[] strings = imageElementContext.getRowStyles();
        for (int i = 0; i < strings.length; i++)
        {
            final String styleName = strings[i];
            final CSSNumericValue value = computeRowHeight(styleName);
            if (value != null)
            {
                calculator.add(value);
            }
        }
        return calculator.getResult();
    }

    protected CSSNumericValue computeRowHeight(final String rowStyle)
    {
        final OfficeStylesCollection contentStyles = getContentStylesCollection();
        final OfficeStyle style = contentStyles.getStyle(OfficeToken.TABLE_ROW, rowStyle);
        if (style != null)
        {
            final Element element = style.getTableRowProperties();
            if (element != null)
            {
                final String height = (String) element.getAttribute(OfficeNamespaces.STYLE_NS, "row-height");
                if (height != null)
                {
                    return parseLength(height);
                }
            }

            final String styleParent = style.getStyleParent();
            if (styleParent != null)
            {
                return computeRowHeight(styleParent);
            }
        }

        final OfficeStylesCollection globalStyles = getGlobalStylesCollection();
        final OfficeStyle globalStyle = globalStyles.getStyle(OfficeToken.TABLE_ROW, rowStyle);
        if (globalStyle != null)
        {
            final Element element = globalStyle.getTableRowProperties();
            if (element != null)
            {
                final String height = (String) element.getAttribute(OfficeNamespaces.STYLE_NS, "row-height");
                if (height != null)
                {
                    return parseLength(height);
                }
            }
            final String styleParent = globalStyle.getStyleParent();
            if (styleParent != null)
            {
                return computeRowHeight(styleParent);
            }
        }

        final OfficeStylesCollection predefStyles = getPredefinedStylesCollection();
        final OfficeStyle predefStyle = predefStyles.getStyle(OfficeToken.TABLE_ROW, rowStyle);
        if (predefStyle != null)
        {
            final Element element = predefStyle.getTableRowProperties();
            if (element != null)
            {
                final String height = (String) element.getAttribute(OfficeNamespaces.STYLE_NS, "row-height");
                if (height != null)
                {
                    return parseLength(height);
                }
            }
            final String styleParent = predefStyle.getStyleParent();
            if (styleParent != null)
            {
                return computeRowHeight(styleParent);
            }
        }
        // not found.
        return null;
    }

    protected CSSNumericValue computeColumnWidth(final String colStyle)
    {
        final OfficeStylesCollection contentStyles = getContentStylesCollection();
        final OfficeStyle style = contentStyles.getStyle(OfficeToken.TABLE_COLUMN, colStyle);
        if (style != null)
        {
            final Element element = style.getTableColumnProperties();
            if (element != null)
            {
                final String height = (String) element.getAttribute(OfficeNamespaces.STYLE_NS, "column-width");
                if (height != null)
                {
                    return parseLength(height);
                }
            }

            final String styleParent = style.getStyleParent();
            if (styleParent != null)
            {
                return computeRowHeight(styleParent);
            }
        }

        final OfficeStylesCollection globalStyles = getGlobalStylesCollection();
        final OfficeStyle globalStyle = globalStyles.getStyle(OfficeToken.TABLE_COLUMN, colStyle);
        if (globalStyle != null)
        {
            final Element element = globalStyle.getTableColumnProperties();
            if (element != null)
            {
                final String height = (String) element.getAttribute(OfficeNamespaces.STYLE_NS, "column-width");
                if (height != null)
                {
                    return parseLength(height);
                }
            }
            final String styleParent = globalStyle.getStyleParent();
            if (styleParent != null)
            {
                return computeRowHeight(styleParent);
            }
        }

        final OfficeStylesCollection predefStyles = getPredefinedStylesCollection();
        final OfficeStyle predefStyle = predefStyles.getStyle(OfficeToken.TABLE_COLUMN, colStyle);
        if (predefStyle != null)
        {
            final Element element = predefStyle.getTableColumnProperties();
            if (element != null)
            {
                final String height = (String) element.getAttribute(OfficeNamespaces.STYLE_NS, "column-width");
                if (height != null)
                {
                    return parseLength(height);
                }
            }
            final String styleParent = predefStyle.getStyleParent();
            if (styleParent != null)
            {
                return computeRowHeight(styleParent);
            }
        }
        // not found.
        return null;
    }

    protected Element produceFirstChild(final Section style,
            final String nameSpace,
            final String type)
    {
        Element paragraphProps = style.findFirstChild(nameSpace, type);
        if (paragraphProps == null)
        {
            paragraphProps = new Section();
            paragraphProps.setNamespace(nameSpace);
            paragraphProps.setType(type);
            style.addNode(paragraphProps);
        }
        return paragraphProps;
    }

    protected void startChartProcessing(final AttributeMap attrs)
            throws ReportProcessingException
    {
        final String classId = (String) attrs.getAttribute(JFreeReportInfo.REPORT_NAMESPACE, "class-id");
        final String chartUrl = (String) attrs.getAttribute(JFreeReportInfo.REPORT_NAMESPACE, "href");
        final ArrayList<?> masterColumns = (ArrayList<?>) attrs.getAttribute(JFreeReportInfo.REPORT_NAMESPACE, SDBCReportDataFactory.MASTER_COLUMNS);
        final ArrayList<?> masterValues = (ArrayList<?>) attrs.getAttribute(JFreeReportInfo.REPORT_NAMESPACE, SDBCReportDataFactory.MASTER_VALUES);
        final ArrayList<?> detailColumns = (ArrayList<?>) attrs.getAttribute(JFreeReportInfo.REPORT_NAMESPACE, SDBCReportDataFactory.DETAIL_COLUMNS);
        final String href = oleProducer.produceOle(chartUrl, masterColumns, masterValues, detailColumns);

        final AttributeList oleList = new AttributeList();
        oleList.setAttribute(OfficeNamespaces.DRAWING_NS, "class-id", classId);
        oleList.setAttribute(OfficeNamespaces.XLINK_NS, "href", "./" + href);
        oleList.setAttribute(OfficeNamespaces.XLINK_NS, "type", "simple");
        oleList.setAttribute(OfficeNamespaces.XLINK_NS, "show", "embed");
        oleList.setAttribute(OfficeNamespaces.XLINK_NS, "actuate", "onLoad");

        try
        {
            getXmlWriter().writeTag(OfficeNamespaces.DRAWING_NS, OfficeToken.OBJECT_OLE, oleList, XmlWriterSupport.CLOSE);
        }
        catch (IOException ioe)
        {
            throw new ReportProcessingException(FAILED, ioe);
        }
    }

    static private double[] calcPaintSize(final CSSNumericValue areaWidth, final CSSNumericValue areaHeight,
            final CSSNumericValue imageWidth, final CSSNumericValue imageHeight)
    {

        final double ratioX = areaWidth.getValue() / imageWidth.getValue();
        final double ratioY = areaHeight.getValue() / imageHeight.getValue();
        final double ratioMin = Math.min(ratioX, ratioY);

        double[] ret = new double[2];
        ret[0] = imageWidth.getValue() * ratioMin;
        ret[1] = imageHeight.getValue() * ratioMin;
        return ret;
    }

    protected void writeNullDate() throws IOException
    {
        // write NULL DATE
        final XmlWriter xmlWriter = getXmlWriter();
        xmlWriter.writeTag(OfficeNamespaces.TABLE_NS, "calculation-settings", null, XmlWriterSupport.OPEN);
        final AttributeMap nullDateAttributes = new AttributeMap();
        nullDateAttributes.setAttribute(OfficeNamespaces.TABLE_NS, "date-value", "1899-12-30");
        xmlWriter.writeTag(OfficeNamespaces.TABLE_NS, "null-date", buildAttributeList(nullDateAttributes), XmlWriterSupport.CLOSE);
        xmlWriter.writeCloseTag();
    }
}
