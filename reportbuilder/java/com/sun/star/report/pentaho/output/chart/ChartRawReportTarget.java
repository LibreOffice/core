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
package com.sun.star.report.pentaho.output.chart;

import com.sun.star.report.DataSourceFactory;
import com.sun.star.report.ImageService;
import com.sun.star.report.InputRepository;
import com.sun.star.report.OfficeToken;
import com.sun.star.report.OutputRepository;
import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.pentaho.PentahoReportEngineMetaData;
import com.sun.star.report.pentaho.output.OfficeDocumentReportTarget;

import java.io.IOException;
import org.jfree.layouting.util.AttributeMap;
import org.jfree.report.DataFlags;
import org.jfree.report.DataSourceException;
import org.jfree.report.ReportProcessingException;
import org.jfree.report.flow.ReportJob;
import org.jfree.report.flow.ReportStructureRoot;
import org.jfree.report.flow.ReportTargetUtil;

import org.pentaho.reporting.libraries.resourceloader.ResourceKey;
import org.pentaho.reporting.libraries.resourceloader.ResourceManager;
import org.pentaho.reporting.libraries.xmlns.common.AttributeList;
import org.pentaho.reporting.libraries.xmlns.writer.XmlWriter;
import org.pentaho.reporting.libraries.xmlns.writer.XmlWriterSupport;

/**
 *
 * @author Ocke Janssen
 */
public class ChartRawReportTarget extends OfficeDocumentReportTarget
{

    private boolean inFilterElements = false;
    private boolean tableRowsStarted = false;
    private int tableCount = 0;
    private int closeTags = 0;

    public ChartRawReportTarget(final ReportJob reportJob,
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
    }

    protected String getTargetMimeType()
    {
        return "application/vnd.oasis.opendocument.chart";
    }

    protected String getStartContent()
    {
        return "chart";
    }

    public String getExportDescriptor()
    {
        return "raw/" + PentahoReportEngineMetaData.OPENDOCUMENT_CHART;
    }

    protected void startContent(final AttributeMap attrs) throws IOException, DataSourceException, ReportProcessingException
    {
        inFilterElements = false;
        closeTags = 0;
        tableCount = 0;
        final XmlWriter xmlWriter = getXmlWriter();
        xmlWriter.writeTag(OfficeNamespaces.OFFICE_NS, getStartContent(), null, XmlWriterSupport.OPEN);
        writeNullDate();
        ++closeTags;
    }

    protected void endContent(final AttributeMap attrs) throws IOException, DataSourceException, ReportProcessingException
    {
        final XmlWriter xmlWriter = getXmlWriter();
        //xmlWriter.writeCloseTag();
        while (closeTags > 0)
        {
            xmlWriter.writeCloseTag();
            --closeTags;
        }
    }

    protected void startReportSection(final AttributeMap attrs, final int role)
            throws IOException, DataSourceException, ReportProcessingException
    {
    }

    protected void endReportSection(final AttributeMap attrs, final int role)
            throws IOException, DataSourceException, ReportProcessingException
    {
    }

    protected void startOther(final AttributeMap attrs) throws IOException, DataSourceException, ReportProcessingException
    {
        final String namespace = ReportTargetUtil.getNamespaceFromAttribute(attrs);
        if (!isFilteredNamespace(namespace))
        {
            final String elementType = ReportTargetUtil.getElemenTypeFromAttribute(attrs);
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

    private boolean isFiltered(final String elementType)
    {
        return OfficeToken.TABLE_HEADER_COLUMNS.equals(elementType) || OfficeToken.TABLE_HEADER_ROWS.equals(elementType) || OfficeToken.TABLE_COLUMNS.equals(elementType);
    }

    protected void endOther(final AttributeMap attrs) throws IOException, DataSourceException, ReportProcessingException
    {
        if (tableRowsStarted && getCurrentRole() == ROLE_TEMPLATE)
        {
            return;
        }
        final String namespace = ReportTargetUtil.getNamespaceFromAttribute(attrs);
        if (!isFilteredNamespace(namespace))
        {
            final String elementType = ReportTargetUtil.getElemenTypeFromAttribute(attrs);
            // if this is the report namespace, write out a table definition ..
            if (OfficeNamespaces.TABLE_NS.equals(namespace))
            {
                if (OfficeToken.TABLE.equals(elementType) || OfficeToken.TABLE_ROWS.equals(elementType))
                {
                    return;
                }
                else if (isFiltered(elementType))
                {
                    inFilterElements = false;
                    if (tableCount > 1)
                    {
                        return;
                    }
                }
            }
            else if (OfficeNamespaces.CHART_NS.equals(namespace) && "chart".equals(elementType))
            {
                return;
            }
            if (inFilterElements && tableCount > 1)
            {
                return;
            }
            final XmlWriter xmlWriter = getXmlWriter();
            xmlWriter.writeCloseTag();
            --closeTags;
        }
    }

    public void processContent(final DataFlags value)
            throws DataSourceException, ReportProcessingException
    {
        if (!(tableRowsStarted && getCurrentRole() == ROLE_TEMPLATE))
        {
            super.processContent(value);
        }
    }

    private void processElement(final AttributeMap attrs, final String namespace, final String elementType)
            throws IOException, ReportProcessingException
    {
        if (tableRowsStarted && getCurrentRole() == ROLE_TEMPLATE)
        {
            return;
        }
        if (OfficeNamespaces.TABLE_NS.equals(namespace))
        {
            if (OfficeToken.TABLE.equals(elementType))
            {
                tableCount += 1;
                if (tableCount > 1)
                {
                    return;
                }
            }
            else if (OfficeToken.TABLE_ROWS.equals(elementType))
            {
                if (tableCount > 1)
                {
                    return;
                }
                tableRowsStarted = true;
            }
            else if (isFiltered(elementType))
            {
                inFilterElements = true;
                if (tableCount > 1)
                {
                    return;
                }
            }
        }
        if (inFilterElements && tableCount > 1)
        {
            return;
        }

        // All styles have to be processed or you will loose the paragraph-styles and inline text-styles.
        // ..
        performStyleProcessing(attrs);

        final AttributeList attrList = buildAttributeList(attrs);
        final XmlWriter xmlWriter = getXmlWriter();
        xmlWriter.writeTag(namespace, elementType, attrList, XmlWriter.OPEN);
        ++closeTags;
        // System.out.println("elementType = " + elementType);
    }
    // /////////////////////////////////////////////////////////////////////////

    public void processText(final String text) throws DataSourceException, ReportProcessingException
    {
        if (inFilterElements && tableCount > 1)
        {
            return;
        }
        super.processText(text);
    }

    public void endReport(final ReportStructureRoot report)
            throws DataSourceException, ReportProcessingException
    {
        super.endReport(report);
        copyMeta();
    }
}
