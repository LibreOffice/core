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
package org.libreoffice.report.pentaho;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.libreoffice.report.DataSourceFactory;
import org.libreoffice.report.ImageService;
import org.libreoffice.report.InputRepository;
import org.libreoffice.report.OutputRepository;
import org.libreoffice.report.ReportEngineMetaData;
import org.libreoffice.report.ReportEngineParameterNames;
import org.libreoffice.report.ReportJobFactory;


public class PentahoReportEngineMetaData
        implements ReportEngineMetaData
{

    public static final String OPENDOCUMENT_TEXT = "application/vnd.oasis.opendocument.text";
    public static final String OPENDOCUMENT_SPREADSHEET = "application/vnd.oasis.opendocument.spreadsheet";
    public static final String OPENDOCUMENT_CHART = "application/vnd.oasis.opendocument.chart";
    public static final String DEBUG = "raw/text+xml";
    private final Map<String,Class<?>> parameterTypes;

    public PentahoReportEngineMetaData()
    {
        parameterTypes = new HashMap<String,Class<?>>();
        parameterTypes.put(ReportEngineParameterNames.CONTENT_TYPE, String.class);
        parameterTypes.put(ReportEngineParameterNames.INPUT_NAME, String.class);
        parameterTypes.put(ReportEngineParameterNames.OUTPUT_NAME, String.class);
        parameterTypes.put(ReportEngineParameterNames.INPUT_REPOSITORY, InputRepository.class);
        parameterTypes.put(ReportEngineParameterNames.OUTPUT_REPOSITORY, OutputRepository.class);
        parameterTypes.put(ReportEngineParameterNames.INPUT_DATASOURCE_FACTORY, DataSourceFactory.class);
        parameterTypes.put(ReportEngineParameterNames.IMAGE_SERVICE, ImageService.class);
        parameterTypes.put(ReportEngineParameterNames.INPUT_REPORTJOB_FACTORY, ReportJobFactory.class);
        parameterTypes.put(ReportEngineParameterNames.INPUT_MASTER_COLUMNS, List.class);
        parameterTypes.put(ReportEngineParameterNames.INPUT_MASTER_VALUES, List.class);
        parameterTypes.put(ReportEngineParameterNames.INPUT_DETAIL_COLUMNS, List.class);
        parameterTypes.put(ReportEngineParameterNames.AUTHOR, String.class);
        parameterTypes.put(ReportEngineParameterNames.TITLE, String.class);
        parameterTypes.put(ReportEngineParameterNames.MAXROWS, Integer.class);
    }

    public Class getParameterType(final String parameter)
    {
        return parameterTypes.get(parameter);
    }

}
