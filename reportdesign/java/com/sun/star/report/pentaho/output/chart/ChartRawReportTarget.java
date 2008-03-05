/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartRawReportTarget.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:35:59 $
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
