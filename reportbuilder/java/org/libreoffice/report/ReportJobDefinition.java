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
package org.libreoffice.report;

/**
 * The report job is created by the report job factory and holds all properties
 * required to complete the reporting task.
 *
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
    ParameterMap getQueryParameters();

    /**
     * The report processing parameters control the behaviour of the report. There are
     * several mandatory parameters, some optional and possibly some not-yet-understood
     * parameters. Use the engine meta data to find out, which parameters are supported.
     *
     * @return the processing parameters
     */
    JobProperties getProcessingParameters();
}
