/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PentahoReportEngine.java,v $
 * $Revision: 1.5 $
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
package com.sun.star.report.pentaho;

import com.sun.star.report.JobDefinitionException;
import com.sun.star.report.ReportEngine;
import com.sun.star.report.ReportEngineMetaData;
import com.sun.star.report.ReportJob;
import com.sun.star.report.ReportJobDefinition;
import com.sun.star.report.util.DefaultReportJobDefinition;
import org.jfree.report.JFreeReportBoot;

public class PentahoReportEngine implements ReportEngine
{

    private final ReportEngineMetaData metaData;

    public PentahoReportEngine()
    {
        JFreeReportBoot.getInstance().start();
        this.metaData = new PentahoReportEngineMetaData();
    }

    public ReportEngineMetaData getMetaData()
    {
        return metaData;
    }

    public ReportJobDefinition createJobDefinition()
    {
        return new DefaultReportJobDefinition(metaData);
    }

    /**
     * Open points: How to define scheduling?
     *
     * @return the report job definition for the job description.
     *
     * @throws com.sun.star.report.JobDefinitionException
     *
     */
    public ReportJob createJob(final ReportJobDefinition definition)
            throws JobDefinitionException
    {
        return new PentahoReportJob(definition);
    }
}
