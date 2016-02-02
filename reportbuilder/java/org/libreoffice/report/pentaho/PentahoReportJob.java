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
package org.libreoffice.report.pentaho;

import org.libreoffice.report.DataSourceFactory;
import org.libreoffice.report.ImageService;
import org.libreoffice.report.InputRepository;
import org.libreoffice.report.JobDefinitionException;
import org.libreoffice.report.JobProperties;
import org.libreoffice.report.OutputRepository;
import org.libreoffice.report.ParameterMap;
import org.libreoffice.report.ReportEngineParameterNames;
import org.libreoffice.report.ReportExecutionException;
import org.libreoffice.report.ReportJob;
import org.libreoffice.report.ReportJobDefinition;
import org.libreoffice.report.SDBCReportDataFactory;
import org.libreoffice.report.pentaho.loader.InputRepositoryLoader;
import org.libreoffice.report.pentaho.model.OfficeDetailSection;
import org.libreoffice.report.pentaho.model.OfficeDocument;
import org.libreoffice.report.pentaho.model.OfficeGroup;
import org.libreoffice.report.pentaho.model.OfficeReport;
import org.libreoffice.report.pentaho.output.chart.ChartRawReportProcessor;
import org.libreoffice.report.pentaho.output.spreadsheet.SpreadsheetRawReportProcessor;
import org.libreoffice.report.pentaho.output.text.TextRawReportProcessor;

import java.io.IOException;

import java.util.ArrayList;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import org.jfree.report.expressions.Expression;
import org.jfree.report.expressions.FormulaExpression;
import org.jfree.report.flow.DefaultReportJob;
import org.jfree.report.flow.ReportProcessor;
import org.jfree.report.flow.raw.XmlPrintReportProcessor;
import org.jfree.report.structure.Node;
import org.jfree.report.structure.Section;
import org.jfree.report.util.ReportParameters;

import org.pentaho.reporting.libraries.formula.lvalues.ContextLookup;
import org.pentaho.reporting.libraries.formula.lvalues.FormulaFunction;
import org.pentaho.reporting.libraries.formula.lvalues.LValue;
import org.pentaho.reporting.libraries.formula.lvalues.Term;
import org.pentaho.reporting.libraries.formula.parser.FormulaParser;
import org.pentaho.reporting.libraries.formula.parser.ParseException;
import org.pentaho.reporting.libraries.resourceloader.Resource;
import org.pentaho.reporting.libraries.resourceloader.ResourceException;
import org.pentaho.reporting.libraries.resourceloader.ResourceManager;


/**
 * ToDo: Allow interrupting of jobs and report the report progress
 */
public class PentahoReportJob implements ReportJob
{

    private static final Log LOGGER = LogFactory.getLog(PentahoReportJob.class);
    private final DataSourceFactory dataSourceFactory;
    private final OutputRepository outputRepository;
    private final JobProperties jobProperties;
    private OfficeDocument report;
    private final ResourceManager resourceManager;
    private final String outputName;
    private final ImageService imageService;
    private final InputRepository inputRepository;
    private final List masterValues;
    private final List detailColumns;

    public PentahoReportJob(final ReportJobDefinition definition)
            throws JobDefinitionException
    {
        if (definition == null)
        {
            throw new NullPointerException();
        }

        this.jobProperties = definition.getProcessingParameters().copy();

        this.dataSourceFactory = (DataSourceFactory) jobProperties.getProperty(ReportEngineParameterNames.INPUT_DATASOURCE_FACTORY);
        if (this.dataSourceFactory == null)
        {
            throw new JobDefinitionException("DataSourceFactory must not be null.");
        }

        this.outputRepository = (OutputRepository) jobProperties.getProperty(ReportEngineParameterNames.OUTPUT_REPOSITORY);
        if (this.outputRepository == null)
        {
            throw new JobDefinitionException("OutputRepository must not be null.");
        }

        this.inputRepository =
                (InputRepository) jobProperties.getProperty(ReportEngineParameterNames.INPUT_REPOSITORY);
        if (inputRepository == null)
        {
            throw new JobDefinitionException("InputRepository must not be null.");
        }

        this.outputName = (String) jobProperties.getProperty(ReportEngineParameterNames.OUTPUT_NAME);
        if (outputName == null)
        {
            throw new JobDefinitionException("OutputName must not be null");
        }

        this.imageService = (ImageService) jobProperties.getProperty(ReportEngineParameterNames.IMAGE_SERVICE);
        if (imageService == null)
        {
            throw new JobDefinitionException("A valid image-service implementation must be given.");
        }

        this.masterValues = (ArrayList<?>) jobProperties.getProperty(ReportEngineParameterNames.INPUT_MASTER_VALUES);
        this.detailColumns = (ArrayList<?>) jobProperties.getProperty(ReportEngineParameterNames.INPUT_DETAIL_COLUMNS);

        this.resourceManager = new ResourceManager();
        this.resourceManager.registerDefaults();
        this.resourceManager.registerLoader(new InputRepositoryLoader(inputRepository));

        try
        {
            this.report = parseReport(definition);
        }
        catch (ResourceException e)
        {
            throw new JobDefinitionException("Failed to parse the report.", e);
        }
    }

    private OfficeDocument parseReport(final ReportJobDefinition definition)
            throws ResourceException, JobDefinitionException
    {
        final String reportResource = (String) this.jobProperties.getProperty(ReportEngineParameterNames.INPUT_NAME);
        if (reportResource == null)
        {
            throw new JobDefinitionException("Report definition name must be given");
        }

        final Resource res = resourceManager.createDirectly("sun:oo://" + reportResource, OfficeDocument.class);
        final OfficeDocument tempReport = (OfficeDocument) res.getResource();
        tempReport.setDataFactory(new StarReportDataFactory(dataSourceFactory));
        tempReport.setJobProperties(definition.getProcessingParameters().copy());
        final ReportParameters inputParameters = tempReport.getInputParameters();

        final ParameterMap queryParameters = definition.getQueryParameters();
        final String[] paramKeys = queryParameters.keys();
        for (int i = 0; i < paramKeys.length; i++)
        {
            final String key = paramKeys[i];
            inputParameters.put(key, queryParameters.get(key));
        }

        return tempReport;
    }

    private void collectGroupExpressions(final Node[] nodes, final List<Object[]> expressions, final FormulaParser parser, final Expression reportFunctions[])
    {
        for (int i = 0; i < nodes.length; i++)
        {
            final Node node = nodes[i];
            if (node instanceof OfficeGroup)
            {
                final OfficeGroup group = (OfficeGroup) node;
                final FormulaExpression exp = (FormulaExpression) group.getGroupingExpression();
                if (exp == null)
                {
                    continue;
                }

                try
                {
                    final String expression = exp.getFormulaExpression();
                    if (expression == null)
                    {
                        continue;
                    }
                    final FormulaFunction function = (FormulaFunction) parser.parse(expression);
                    final LValue[] parameters = function.getChildValues();
                    if (parameters.length > 0)
                    {
                        String name = parameters[0].toString();
                        if (parameters[0] instanceof ContextLookup)
                        {
                            final ContextLookup context = (ContextLookup) parameters[0];
                            name = context.getName();
                        }
                        for (int j = 0; j < reportFunctions.length; j++)
                        {
                            if (reportFunctions[j] instanceof FormulaExpression)
                            {
                                final FormulaExpression reportExp = (FormulaExpression) reportFunctions[j];

                                if (reportExp.getName().equals(name))
                                {
                                    LValue val = parser.parse(reportExp.getFormulaExpression());
                                    while( !(val instanceof ContextLookup))
                                    {
                                        if (val instanceof Term)
                                        {
                                            val = ((Term)val).getHeadValue();
                                        }
                                        else if (val instanceof FormulaFunction)
                                        {
                                            final FormulaFunction reportFunction = (FormulaFunction) val;
                                            val = reportFunction.getChildValues()[0];
                                        }
                                    }
                                    final ContextLookup context = (ContextLookup) val;
                                    name = context.getName();
                                    break;
                                }
                            }
                        }

                        final Object[] pair = new Object[2];
                        pair[0] = name;
                        pair[1] = group.getAttribute(OfficeNamespaces.OOREPORT_NS, "sort-ascending");
                        expressions.add(pair);
                    }
                }
                catch (ParseException ex)
                {
                    LOGGER.error("ReportProcessing failed", ex);
                }
            }
            else if (node instanceof OfficeDetailSection)
            {
                return;
            }
            if (node instanceof Section)
            {
                final Section section = (Section) node;
                collectGroupExpressions(section.getNodeArray(), expressions, parser, reportFunctions);
            }
        }
    }

    private void collectSortExpressions(final Node[] nodes, final List<Object[]> expressions, final FormulaParser parser, final Expression reportFunctions[])
    {
        for (int i = 0; i < nodes.length; i++)
        {
            final Node node = nodes[i];
            if (node instanceof OfficeGroup)
            {
                final OfficeGroup group = (OfficeGroup) node;
                final String exp = group.getSortingExpression();
                if (exp == null)
                {
                    continue;
                }

                final Object[] pair = new Object[2];
                pair[0] = exp;
                pair[1] = group.getAttribute(OfficeNamespaces.OOREPORT_NS, "sort-ascending");
                expressions.add(pair);
            }
            else if (node instanceof OfficeDetailSection)
            {
                return;
            }
            if (node instanceof Section)
            {
                final Section section = (Section) node;
                collectSortExpressions(section.getNodeArray(), expressions, parser, reportFunctions);
            }
        }
    }

    private void setMetaDataProperties(DefaultReportJob job)
    {
        job.getConfiguration().setConfigProperty(ReportEngineParameterNames.AUTHOR, (String) jobProperties.getProperty(ReportEngineParameterNames.AUTHOR));
        job.getConfiguration().setConfigProperty(ReportEngineParameterNames.TITLE, (String) jobProperties.getProperty(ReportEngineParameterNames.TITLE));
    }

    /**
     * Although we might want to run the job as soon as it has been created, sometimes it is
     * wiser to let the user add some listeners first. If we execute at once, the user
     * either has to deal with threading code or won't receive any progress information in
     * single threaded environments.
     */
    public void execute()
            throws ReportExecutionException, IOException
    {
        final DefaultReportJob job = new DefaultReportJob(report);
        setMetaDataProperties(job);
        final String contentType = (String) jobProperties.getProperty(ReportEngineParameterNames.CONTENT_TYPE);
        //noinspection OverlyBroadCatchBlock
        try
        {
            final ReportParameters parameters = job.getParameters();

            if (masterValues != null && detailColumns != null)
            {
                parameters.put(SDBCReportDataFactory.MASTER_VALUES, masterValues);
                parameters.put(SDBCReportDataFactory.DETAIL_COLUMNS, detailColumns);
            }

            final Node[] nodes = report.getNodeArray();

            final FormulaParser parser = new FormulaParser();
            final OfficeReport officeReport = (OfficeReport) ((Section) nodes[0]).getNode(0);
            final Section reportBody = (Section) officeReport.getBodySection();
            final ArrayList<Object[]> sortExpressions = new ArrayList<Object[]>();
            collectSortExpressions(reportBody.getNodeArray(), sortExpressions, parser, officeReport.getExpressions());
            parameters.put(SDBCReportDataFactory.SORT_EXPRESSIONS, sortExpressions);
            final ArrayList<Object[]> groupExpressions = new ArrayList<Object[]>();
            collectGroupExpressions(reportBody.getNodeArray(), groupExpressions, parser, officeReport.getExpressions());
            parameters.put(SDBCReportDataFactory.GROUP_EXPRESSIONS, groupExpressions);
            final String command = (String) officeReport.getAttribute(OfficeNamespaces.OOREPORT_NS, "command");
            final String commandType = (String) officeReport.getAttribute(OfficeNamespaces.OOREPORT_NS, SDBCReportDataFactory.COMMAND_TYPE);
            final String escapeProcessing = (String) officeReport.getAttribute(OfficeNamespaces.OOREPORT_NS, SDBCReportDataFactory.ESCAPE_PROCESSING);
            report.setQuery(command);
            parameters.put(SDBCReportDataFactory.COMMAND_TYPE, commandType);
            parameters.put(SDBCReportDataFactory.ESCAPE_PROCESSING, !("false".equals(escapeProcessing)));

            final String filter = (String) officeReport.getAttribute(OfficeNamespaces.OOREPORT_NS, "filter");
            parameters.put(SDBCReportDataFactory.UNO_FILTER, filter);

            parameters.put(ReportEngineParameterNames.MAXROWS, report.getJobProperties().getProperty(ReportEngineParameterNames.MAXROWS));

            final long startTime = System.currentTimeMillis();
            final ReportProcessor rp = getProcessorForContentType(contentType);
            rp.processReport(job);
            job.close();
            final long endTime = System.currentTimeMillis();
            LOGGER.debug("Report processing time: " + (endTime - startTime));
        }
        catch (final Exception e)
        {
            String message = e.getMessage();
            if (message == null || message.length() == 0)
            {
                message = "Failed to process the report";
            }
            throw new ReportExecutionException(message, e);
        }

    }

    private ReportProcessor getProcessorForContentType(final String mimeType)
            throws ReportExecutionException
    {
        final ReportProcessor ret;

        if (PentahoReportEngineMetaData.OPENDOCUMENT_SPREADSHEET.equals(mimeType))
        {
            ret = new SpreadsheetRawReportProcessor(inputRepository, outputRepository, outputName, imageService, dataSourceFactory);
        }
        else if (PentahoReportEngineMetaData.OPENDOCUMENT_TEXT.equals(mimeType))
        {
            ret = new TextRawReportProcessor(inputRepository, outputRepository, outputName, imageService, dataSourceFactory);
        }
        else if (PentahoReportEngineMetaData.OPENDOCUMENT_CHART.equals(mimeType))
        {
            ret = new ChartRawReportProcessor(inputRepository, outputRepository, outputName, imageService, dataSourceFactory);
        }
        else if (PentahoReportEngineMetaData.DEBUG.equals(mimeType))
        {
            ret = new XmlPrintReportProcessor(System.out, "ISO-8859-1");
        }
        else
        {
            throw new ReportExecutionException("Invalid mime-type");
        }

        return ret;
    }
}
