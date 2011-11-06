/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
