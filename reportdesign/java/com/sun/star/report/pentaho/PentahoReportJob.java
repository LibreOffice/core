/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PentahoReportJob.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:30:00 $
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
package com.sun.star.report.pentaho;

import java.io.IOException;
import java.util.ArrayList;

import com.sun.star.report.DataSourceFactory;
import com.sun.star.report.InputRepository;
import com.sun.star.report.JobDefinitionException;
import com.sun.star.report.JobProgressIndicator;
import com.sun.star.report.JobProperties;
import com.sun.star.report.OutputRepository;
import com.sun.star.report.ParameterMap;
import com.sun.star.report.ReportEngineParameterNames;
import com.sun.star.report.ReportExecutionException;
import com.sun.star.report.ReportJob;
import com.sun.star.report.ReportJobDefinition;
import com.sun.star.report.ImageService;
import com.sun.star.report.pentaho.loader.InputRepositoryLoader;
import com.sun.star.report.pentaho.model.OfficeDocument;
import com.sun.star.report.pentaho.model.OfficeGroup;
import com.sun.star.report.pentaho.model.OfficeReport;
import com.sun.star.report.pentaho.output.chart.ChartRawReportProcessor;
import com.sun.star.report.pentaho.output.text.TextRawReportProcessor;
import com.sun.star.report.pentaho.output.spreadsheet.SpreadsheetRawReportProcessor;
import java.util.Vector;
import org.jfree.report.expressions.FormulaExpression;
import org.jfree.report.flow.DefaultReportJob;
import org.jfree.report.flow.ReportProcessor;
import org.jfree.report.flow.raw.XmlPrintReportProcessor;
import org.jfree.report.structure.Node;
import org.jfree.report.structure.Section;
import org.jfree.report.util.ReportParameters;
import org.jfree.resourceloader.Resource;
import org.jfree.resourceloader.ResourceException;
import org.jfree.resourceloader.ResourceManager;
import org.jfree.util.Log;

/**
 * ToDo: Allow interrupting of jobs and report the report progress
 */
public class PentahoReportJob implements ReportJob
{

    private boolean finished;
    private ArrayList listeners;
    private DataSourceFactory dataSourceFactory;
    private OutputRepository outputRepository;
    private JobProperties jobProperties;
    private OfficeDocument report;
    private ResourceManager resourceManager;
    private String outputName;
    private ImageService imageService;
    private InputRepository inputRepository;
    private ReportJobDefinition definition;

    public ReportJobDefinition getDefinition()
    {
        return definition;
    }

    public PentahoReportJob(final ReportJobDefinition definition)
            throws JobDefinitionException
    {
        if (definition == null)
        {
            throw new NullPointerException();
        }

        this.definition = definition;
        this.listeners = new ArrayList();
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

    public void addProgressIndicator(final JobProgressIndicator indicator)
    {
        listeners.add(indicator);
    }

    /**
     * Interrupt the job.
     */
    public void interrupt()
    {
    // hey, not yet ..
    }

    /**
     * Queries the jobs result status.
     *
     * @return true, if the job is finished (or has been interrupted), false if the job
     *         waits for activation.
     */
    public boolean isFinished()
    {
        return finished;
    }

    public void finish()
    {
        finished = true;
    }

    /**
     * Queries the jobs execution status.
     *
     * @return true, if the job is currently running, false otherwise.
     */
    public boolean isRunning()
    {
        return finished == false;
    }

    public void removeProgressIndicator(final JobProgressIndicator indicator)
    {
        listeners.remove(indicator);
    }

    private void collectGroupExpressions(Node[] nodes, Vector expressions, OfficeReport[] report)
    {
        for (int i = 0; i < nodes.length; i++)
        {
            Node node = nodes[i];
            if (node instanceof OfficeGroup)
            {
                OfficeGroup group = (OfficeGroup) node;
                FormulaExpression exp = (FormulaExpression) group.getGroupingExpression();
                Object[] pair = new Object[2];
                pair[0] = exp.getFormula();
                pair[1] = group.getAttribute(OfficeNamespaces.OOREPORT_NS, "sort-ascending");
                expressions.add(pair);

            }
            else if (node instanceof OfficeReport && report[0] == null)
            {
                report[0] = (OfficeReport) node;
            }
            if (node instanceof Section)
            {
                Section section = (Section) node;
                collectGroupExpressions(section.getNodeArray(), expressions, report);
            }

        }
    }

    /**
     * Although we might want to run the job as soon as it has been created, sometimes it is
     * wiser to let the user add some listeners first. If we execute at once, the user
     * either has to deal with threading code or wont receive any progress information in
     * single threaded environments.
     */
    public void execute()
            throws ReportExecutionException, IOException
    {
        final DefaultReportJob job = new DefaultReportJob(report);

        final String contentType = (String) jobProperties.getProperty(ReportEngineParameterNames.CONTENT_TYPE);
        //noinspection OverlyBroadCatchBlock
        try
        {
            ReportParameters parameters = job.getParameters();
            Node[] nodes = report.getNodeArray();
            Vector expressions = new Vector();
            OfficeReport[] officeReport = new OfficeReport[1];
            collectGroupExpressions(nodes, expressions, officeReport);
            parameters.put("group-expressions", expressions);
            String command = (String) officeReport[0].getAttribute(OfficeNamespaces.OOREPORT_NS, "command");
            String commandType = (String) officeReport[0].getAttribute(OfficeNamespaces.OOREPORT_NS, "command-type");
            report.setQuery(command);
            parameters.put("command-type", commandType);

            final long startTime = System.currentTimeMillis();
            final ReportProcessor rp = getProcessorForContentType(contentType);
            rp.processReport(job);
            job.close();
            final long endTime = System.currentTimeMillis();
            Log.debug("Report processing time: " + (endTime - startTime));
        }
        catch (final Exception e)
        {
            throw new ReportExecutionException("Failed to process the report", e);
        }

    }

    protected ReportProcessor getProcessorForContentType(final String mimeType)
            throws ReportExecutionException
    {
        ReportProcessor ret = null;
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
        if (ret == null)
        {
            throw new ReportExecutionException("Invalid mime-type");
        }
        return ret;
    }
}
