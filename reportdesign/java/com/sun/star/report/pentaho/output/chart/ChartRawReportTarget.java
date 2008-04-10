/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ChartRawReportTarget.java,v $
 * $Revision: 1.3 $
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
package com.sun.star.report.pentaho.output.chart;

import com.sun.star.report.DataSourceFactory;
import com.sun.star.report.ImageService;
import com.sun.star.report.InputRepository;
import com.sun.star.report.OutputRepository;
import com.sun.star.report.pentaho.PentahoReportEngineMetaData;
import com.sun.star.report.pentaho.output.spreadsheet.SpreadsheetRawReportTarget;
import org.jfree.report.ReportProcessingException;
import org.jfree.report.flow.ReportJob;
import org.jfree.resourceloader.ResourceKey;
import org.jfree.resourceloader.ResourceManager;

/**
 *
 * @author Ocke Janssen
 */
public class ChartRawReportTarget extends SpreadsheetRawReportTarget
{

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
}
