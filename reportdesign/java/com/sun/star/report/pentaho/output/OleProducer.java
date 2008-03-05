/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OleProducer.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:35:16 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
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
import java.io.IOException;
import java.util.Vector;

/**
 *
 * @author Ocke Janssen
 */
public class OleProducer {

    private InputRepository inputRepository;
    private OutputRepository outputRepository;
    private DefaultNameGenerator nameGenerator;
    private DataSourceFactory dataSourceFactory;
    private ImageService imageService;


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

    String produceOle(final String source,final Vector masterColumns,final Vector masterValues,final Vector detailColumns) {
        InputRepository subInputRepository = null;
        OutputRepository subOutputRepository = null;
        String output = "";
        try {
            subInputRepository = inputRepository.openInputRepository(source);
            output = nameGenerator.generateStorageName("Object ", null);
            subOutputRepository = outputRepository.openOutputRepository(output);
            try {

                final PentahoReportEngine engine = new PentahoReportEngine();
                final ReportJobDefinition definition = engine.createJobDefinition();
                final JobProperties procParms = definition.getProcessingParameters();

                procParms.setProperty(ReportEngineParameterNames.INPUT_REPOSITORY, subInputRepository);
                procParms.setProperty(ReportEngineParameterNames.OUTPUT_REPOSITORY, subOutputRepository);
                procParms.setProperty(ReportEngineParameterNames.INPUT_NAME, "content.xml");
                procParms.setProperty(ReportEngineParameterNames.OUTPUT_NAME, "content.xml");
                procParms.setProperty(ReportEngineParameterNames.CONTENT_TYPE, "application/vnd.oasis.opendocument.chart");
                procParms.setProperty(ReportEngineParameterNames.INPUT_DATASOURCE_FACTORY, dataSourceFactory);
                procParms.setProperty(ReportEngineParameterNames.INPUT_MASTER_COLUMNS, masterColumns);
                procParms.setProperty(ReportEngineParameterNames.INPUT_MASTER_VALUES, masterValues);
                procParms.setProperty(ReportEngineParameterNames.INPUT_DETAIL_COLUMNS, detailColumns);
                procParms.setProperty(ReportEngineParameterNames.IMAGE_SERVICE, imageService);

                engine.createJob(definition).execute();
            } catch (ReportExecutionException ex) {
                ex.printStackTrace();
            } catch (IOException ex) {
                ex.printStackTrace();
            }
        } catch (IOException ex) {
            ex.printStackTrace();
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
