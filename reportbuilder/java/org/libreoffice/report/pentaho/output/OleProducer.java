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
import org.libreoffice.report.JobProperties;
import org.libreoffice.report.OutputRepository;
import org.libreoffice.report.ReportEngineParameterNames;
import org.libreoffice.report.ReportExecutionException;
import org.libreoffice.report.ReportJobDefinition;
import org.libreoffice.report.pentaho.DefaultNameGenerator;
import org.libreoffice.report.pentaho.PentahoReportEngine;
import org.libreoffice.report.pentaho.PentahoReportEngineMetaData;

import java.io.IOException;

import java.util.List;
import java.util.logging.Logger;

public class OleProducer
{

    private static final Logger LOGGER = Logger.getLogger(OleProducer.class.getName());
    private final InputRepository inputRepository;
    private final OutputRepository outputRepository;
    private final DefaultNameGenerator nameGenerator;
    private final DataSourceFactory dataSourceFactory;
    private final ImageService imageService;
    private final Integer maxRows;

    public OleProducer(final InputRepository inputRepository,
            final OutputRepository outputRepository, final ImageService imageService, final DataSourceFactory dataSourceFactory, final Integer maxRows)
    {
        if (inputRepository == null)
        {
            throw new NullPointerException();
        }
        if (outputRepository == null)
        {
            throw new NullPointerException();
        }

        this.inputRepository = inputRepository;
        this.outputRepository = outputRepository;
        this.nameGenerator = new DefaultNameGenerator(outputRepository);
        this.dataSourceFactory = dataSourceFactory;
        this.imageService = imageService;
        this.maxRows = maxRows;
    }

    String produceOle(final String source, final List masterColumns, final List masterValues, final List detailColumns)
    {
        InputRepository subInputRepository = null;
        OutputRepository subOutputRepository = null;
        String output = "";
        try
        {
            subInputRepository = inputRepository.openInputRepository(source);
            output = nameGenerator.generateStorageName("Object", null);
            subOutputRepository = outputRepository.openOutputRepository(output, PentahoReportEngineMetaData.OPENDOCUMENT_CHART);
            try
            {

                final PentahoReportEngine engine = new PentahoReportEngine();
                final ReportJobDefinition definition = engine.createJobDefinition();
                final JobProperties procParms = definition.getProcessingParameters();

                procParms.setProperty(ReportEngineParameterNames.INPUT_REPOSITORY, subInputRepository);
                procParms.setProperty(ReportEngineParameterNames.OUTPUT_REPOSITORY, subOutputRepository);
                procParms.setProperty(ReportEngineParameterNames.INPUT_NAME, "content.xml");
                procParms.setProperty(ReportEngineParameterNames.OUTPUT_NAME, "content.xml");
                procParms.setProperty(ReportEngineParameterNames.CONTENT_TYPE, PentahoReportEngineMetaData.OPENDOCUMENT_CHART);
                procParms.setProperty(ReportEngineParameterNames.INPUT_DATASOURCE_FACTORY, dataSourceFactory);
                procParms.setProperty(ReportEngineParameterNames.INPUT_MASTER_COLUMNS, masterColumns);
                procParms.setProperty(ReportEngineParameterNames.INPUT_MASTER_VALUES, masterValues);
                procParms.setProperty(ReportEngineParameterNames.INPUT_DETAIL_COLUMNS, detailColumns);
                procParms.setProperty(ReportEngineParameterNames.IMAGE_SERVICE, imageService);
                procParms.setProperty(ReportEngineParameterNames.MAXROWS, maxRows);

                engine.createJob(definition).execute();
            }
            catch (ReportExecutionException ex)
            {
                LOGGER.severe("ReportProcessing failed: " + ex);
            }
            catch (IOException ex)
            {
                LOGGER.severe("ReportProcessing failed: " + ex);
            }
        }
        catch (IOException ex)
        {
            LOGGER.severe("ReportProcessing failed: " + ex);
        } finally
        {
            if (subInputRepository != null)
            {
                subInputRepository.closeInputRepository();
            }
            if (subOutputRepository != null)
            {
                subOutputRepository.closeOutputRepository();
            }
        }
        return output;
    }
}
