/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SpreadsheetRawReportProcessor.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: oj $ $Date: 2007-08-03 10:35:05 $
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

package com.sun.star.report.pentaho.output.spreadsheet;

import org.jfree.report.DataSourceException;
import org.jfree.report.ReportDataFactoryException;
import org.jfree.report.ReportProcessingException;
import org.jfree.report.flow.AbstractReportProcessor;
import org.jfree.report.flow.ReportJob;
import org.jfree.report.flow.ReportStructureRoot;
import org.jfree.report.flow.ReportTarget;
import org.jfree.resourceloader.ResourceManager;
import com.sun.star.report.OutputRepository;
import com.sun.star.report.InputRepository;
import com.sun.star.report.ImageService;

/**
 * @author Michael D'Amour
 */
public class SpreadsheetRawReportProcessor extends AbstractReportProcessor
{
  private OutputRepository outputRepository;

  private String targetName;
  private InputRepository inputRepository;
  private ImageService imageService;

  public SpreadsheetRawReportProcessor(final InputRepository inputRepository,
                                       final OutputRepository outputRepository,
                                       final String targetName,
                                       final ImageService imageService)
  {
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
    if (inputRepository == null)
    {
      throw new NullPointerException();
    }
    this.targetName = targetName;
    this.inputRepository = inputRepository;
    this.outputRepository = outputRepository;
    this.imageService = imageService;
  }

  protected ReportTarget createReportTarget(final ReportJob job) throws ReportProcessingException
  {
    final ReportStructureRoot report = job.getReportStructureRoot();
    final ResourceManager resourceManager = report.getResourceManager();
    return new SpreadsheetRawReportTarget
        (job, resourceManager, report.getBaseResource(), inputRepository, outputRepository, targetName, imageService);
  }

  public void processReport(final ReportJob job) throws ReportDataFactoryException, DataSourceException,
      ReportProcessingException
  {
    final ReportTarget reportTarget = createReportTarget(job);
    // first run: collect table cell sizes for all tables
    processReportRun(job, reportTarget);
    // second run: uses table cell data to output a single uniform table
    processReportRun(job, reportTarget);
  }
}
