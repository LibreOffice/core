/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


package com.sun.star.report.pentaho.output.chart;

import com.sun.star.report.DataSourceFactory;
import com.sun.star.report.ImageService;
import com.sun.star.report.InputRepository;
import com.sun.star.report.OutputRepository;

import org.jfree.report.DataSourceException;
import org.jfree.report.ReportDataFactoryException;
import org.jfree.report.ReportProcessingException;
import org.jfree.report.flow.ReportJob;
import org.jfree.report.flow.ReportStructureRoot;
import org.jfree.report.flow.ReportTarget;
import org.jfree.report.flow.SinglePassReportProcessor;

import org.pentaho.reporting.libraries.resourceloader.ResourceManager;

/**
 *
 * @author Ocke Janssen
 */
public class ChartRawReportProcessor extends SinglePassReportProcessor
{

    private final OutputRepository outputRepository;
    private final String targetName;
    private final InputRepository inputRepository;
    private final ImageService imageService;
    private final DataSourceFactory dataSourceFactory;

    public ChartRawReportProcessor(final InputRepository inputRepository,
            final OutputRepository outputRepository,
            final String targetName,
            final ImageService imageService,
            final DataSourceFactory dataSourceFactory)
    {
        if (inputRepository == null)
        {
            throw new NullPointerException();
        }
        if (outputRepository == null)
        {
            throw new NullPointerException();
        }
        if (targetName == null)
        {
            throw new NullPointerException();
        }
        if (imageService == null)
        {
            throw new NullPointerException();
        }
        if (dataSourceFactory == null)
        {
            throw new NullPointerException();
        }
        this.targetName = targetName;
        this.inputRepository = inputRepository;
        this.outputRepository = outputRepository;
        this.imageService = imageService;
        this.dataSourceFactory = dataSourceFactory;
    }

    protected ReportTarget createReportTarget(final ReportJob job)
            throws ReportProcessingException
    {
        final ReportStructureRoot report = job.getReportStructureRoot();
        final ResourceManager resourceManager = report.getResourceManager();

        return new ChartRawReportTarget(job, resourceManager, report.getBaseResource(),
                inputRepository, outputRepository, targetName, imageService, dataSourceFactory);
    }

    public void processReport(final ReportJob job) throws ReportDataFactoryException, DataSourceException,
            ReportProcessingException
    {
        final ReportTarget reportTarget = createReportTarget(job);
        processReportRun(job, reportTarget);
    }
}
