/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OleProducer.java,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
package com.sun.star.report.pentaho.output;

import com.sun.star.report.DataSourceFactory;
import com.sun.star.report.ImageService;
import com.sun.star.report.InputRepository;
import com.sun.star.report.JobProperties;
import com.sun.star.report.OutputRepository;
import com.sun.star.report.ReportEngineParameterNames;
import com.sun.star.report.ReportExecutionException;
import com.sun.star.report.ReportJobDefinition;
import com.sun.star.report.pentaho.DefaultNameGenerator;
import com.sun.star.report.pentaho.PentahoReportEngine;
import com.sun.star.report.pentaho.PentahoReportEngineMetaData;
import java.io.IOException;
import java.util.List;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 *
 * @author Ocke Janssen
 */
public class OleProducer {

    private static final Log LOGGER = LogFactory.getLog(OleProducer.class);
    private final InputRepository inputRepository;
    private final OutputRepository outputRepository;
    private final DefaultNameGenerator nameGenerator;
    private final DataSourceFactory dataSourceFactory;
    private final ImageService imageService;


    public OleProducer(final InputRepository inputRepository,
            final OutputRepository outputRepository,final ImageService imageService,final DataSourceFactory dataSourceFactory) {
        if (inputRepository == null) {
            throw new NullPointerException();
        }
        if (outputRepository == null) {
            throw new NullPointerException();
        }

        this.inputRepository = inputRepository;
        this.outputRepository = outputRepository;
        this.nameGenerator = new DefaultNameGenerator(outputRepository);
        this.dataSourceFactory = dataSourceFactory;
        this.imageService = imageService;
    }

    String produceOle(final String source,final List masterColumns,final List masterValues,final List detailColumns) {
        InputRepository subInputRepository = null;
        OutputRepository subOutputRepository = null;
        String output = "";
        try {
            subInputRepository = inputRepository.openInputRepository(source);
            output = nameGenerator.generateStorageName("Object", null);
            subOutputRepository = outputRepository.openOutputRepository(output, PentahoReportEngineMetaData.OPENDOCUMENT_CHART);
            try {

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

                engine.createJob(definition).execute();
            } catch (ReportExecutionException ex) {
                LOGGER.error("ReportProcessing failed", ex);
            } catch (IOException ex) {
                LOGGER.error("ReportProcessing failed", ex);
            }
        } catch (IOException ex) {
            LOGGER.error("ReportProcessing failed", ex);
        } finally {
            if (subInputRepository != null) {
                subInputRepository.closeInputRepository();
            }
            if (subOutputRepository != null) {
                subOutputRepository.closeOutputRepository();
            }
        }
        return output;
    }
}
