/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: TextRawReportProcessor.java,v $
 * $Revision: 1.6 $
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
package com.sun.star.report.pentaho.output.text;

import com.sun.star.report.DataSourceFactory;
import com.sun.star.report.OutputRepository;
import com.sun.star.report.InputRepository;
import com.sun.star.report.ImageService;
import com.sun.star.report.pentaho.PentahoFormulaContext;
import org.jfree.report.ReportProcessingException;
import org.jfree.report.data.ReportContextImpl;
import org.jfree.report.flow.ReportContext;
import org.jfree.report.flow.ReportJob;
import org.jfree.report.flow.ReportStructureRoot;
import org.jfree.report.flow.ReportTarget;
import org.jfree.report.flow.SinglePassReportProcessor;
import org.pentaho.reporting.libraries.resourceloader.ResourceManager;

/**
 * Creation-Date: 03.07.2006, 17:08:25
 *
 * @author Thomas Morgner
 */
public class TextRawReportProcessor extends SinglePassReportProcessor
{

    private final OutputRepository outputRepository;
    private final String targetName;
    private final InputRepository inputRepository;
    private final ImageService imageService;
    private final DataSourceFactory dataSourceFactory;

    public TextRawReportProcessor(final InputRepository inputRepository,
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

        return new TextRawReportTarget(job, resourceManager, report.getBaseResource(),
                inputRepository, outputRepository, targetName, imageService, dataSourceFactory);
    }

    protected ReportContext createReportContext(final ReportJob job,
            final ReportTarget target)
    {
        final ReportContext context = super.createReportContext(job, target);
        if (context instanceof ReportContextImpl)
        {
            final ReportContextImpl impl = (ReportContextImpl) context;
            impl.setFormulaContext(new PentahoFormulaContext(impl.getFormulaContext(),job.getConfiguration()));
        }
        return context;
    }
}


