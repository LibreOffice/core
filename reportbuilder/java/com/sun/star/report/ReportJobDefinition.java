/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ReportJobDefinition.java,v $
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
 * The report job is created by the report job factory and holds all properties
 * required to complete the reporting task.
 *
 * @author Thomas Morgner
 */
public interface ReportJobDefinition
{

    /**
     * The parameters of the root report definition. The report parameters are using
     * by the query factory to parametrize the query statement.
     *
     * The query parameters for the subreports are defined using mappings, it would not
     * make sense to define them here.
     *
     * @return a map containing the report parameters
     */
    public ParameterMap getQueryParameters();

    /**
     * The report processing parameters control the behaviour of the report. There are
     * several mandatory parameters, some optional and possibly some not-yet-understood
     * parameters. Use the engine meta data to find out, which parameters are supported.
     *
     * @return the processing parameters
     */
    public JobProperties getProcessingParameters();
}
