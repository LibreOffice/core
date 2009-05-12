/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ReportEngine.java,v $
 * $Revision: 1.4 $
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
 * @author Thomas Morgner
 */
public interface ReportEngine
{

    public ReportJobDefinition createJobDefinition();

    public ReportEngineMetaData getMetaData();

    /**
     * Open points: How to define scheduling?
     *
     * @return the report job definition for the job description.
     * @throws JobDefinitionException
     */
    public ReportJob createJob(ReportJobDefinition definition)
            throws JobDefinitionException;
}
