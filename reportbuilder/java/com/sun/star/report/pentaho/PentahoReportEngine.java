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
package com.sun.star.report.pentaho;

import com.sun.star.report.JobDefinitionException;
import com.sun.star.report.ReportEngine2;
import com.sun.star.report.ReportEngineMetaData;
import com.sun.star.report.ReportJob;
import com.sun.star.report.ReportJobDefinition;
import com.sun.star.report.util.DefaultReportJobDefinition;
import org.jfree.report.JFreeReportBoot;

public class PentahoReportEngine implements ReportEngine2
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
