/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PentahoReportEngine.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:29:24 $
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

import com.sun.star.report.JobDefinitionException;
import com.sun.star.report.ReportEngine;
import com.sun.star.report.ReportEngineMetaData;
import com.sun.star.report.ReportJob;
import com.sun.star.report.ReportJobDefinition;
import com.sun.star.report.util.DefaultReportJobDefinition;
import org.jfree.report.JFreeReportBoot;

public class PentahoReportEngine implements ReportEngine
{

    private ReportEngineMetaData metaData;

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
