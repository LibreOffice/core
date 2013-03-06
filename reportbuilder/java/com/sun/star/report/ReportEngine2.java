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
package com.sun.star.report;

/**
 * The job factory collects all required properties to build a reportJob
 * object. Implementors should define a suitable set of properties to allow
 * the configuration of the created report jobs.
 *
 * How these properties are collected is beyond the scope of this interface.
 * For each type of Job, there should be separate job-factory (remote jobs
 * vs. local jobs etc).
 *
 */
public interface ReportEngine2
{

    public ReportJobDefinition createJobDefinition();

    public ReportEngineMetaData getMetaData();

    /**
     * Open points: How to define scheduling?
     *
     * @param definition
     * @return the report job definition for the job description.
     * @throws JobDefinitionException
     */
    public ReportJob createJob(ReportJobDefinition definition)
            throws JobDefinitionException;
}
