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

import com.sun.star.report.DataSourceFactory;
import com.sun.star.report.ImageService;
import com.sun.star.report.InputRepository;
import com.sun.star.report.OutputRepository;
import com.sun.star.report.ReportEngineMetaData;
import com.sun.star.report.ReportEngineParameterNames;
import com.sun.star.report.ReportJobFactory;

import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.pentaho.reporting.libraries.base.util.HashNMap;


public class PentahoReportEngineMetaData
        implements ReportEngineMetaData
{

    public static final String OPENDOCUMENT_TEXT = "application/vnd.oasis.opendocument.text";
    public static final String OPENDOCUMENT_SPREADSHEET = "application/vnd.oasis.opendocument.spreadsheet";
    public static final String OPENDOCUMENT_CHART = "application/vnd.oasis.opendocument.chart";
    public final static String CONTENT_TYPE = "content-type";
    public static final String DEBUG = "raw/text+xml";
    private final Set mandatoryParameters;
    private final Map parameterTypes;
    private final HashNMap enumerationValues;

    public PentahoReportEngineMetaData()
    {
        mandatoryParameters = new HashSet();
        mandatoryParameters.add(ReportEngineParameterNames.CONTENT_TYPE);
        mandatoryParameters.add(ReportEngineParameterNames.INPUT_NAME);
        mandatoryParameters.add(ReportEngineParameterNames.INPUT_REPOSITORY);
        mandatoryParameters.add(ReportEngineParameterNames.OUTPUT_NAME);
        mandatoryParameters.add(ReportEngineParameterNames.OUTPUT_REPOSITORY);
        mandatoryParameters.add(ReportEngineParameterNames.INPUT_DATASOURCE_FACTORY);
        mandatoryParameters.add(ReportEngineParameterNames.IMAGE_SERVICE);
        mandatoryParameters.add(ReportEngineParameterNames.INPUT_REPORTJOB_FACTORY);
        mandatoryParameters.add(ReportEngineParameterNames.INPUT_MASTER_COLUMNS);
        mandatoryParameters.add(ReportEngineParameterNames.INPUT_MASTER_VALUES);
        mandatoryParameters.add(ReportEngineParameterNames.INPUT_DETAIL_COLUMNS);
        mandatoryParameters.add(ReportEngineParameterNames.AUTHOR);
        mandatoryParameters.add(ReportEngineParameterNames.TITLE);
        mandatoryParameters.add(ReportEngineParameterNames.MAXROWS);


        parameterTypes = new HashMap();
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

        enumerationValues = new HashNMap();
        enumerationValues.add(CONTENT_TYPE,
                PentahoReportEngineMetaData.OPENDOCUMENT_TEXT);
        enumerationValues.add(CONTENT_TYPE,
                PentahoReportEngineMetaData.OPENDOCUMENT_SPREADSHEET);
        enumerationValues.add(CONTENT_TYPE,
                PentahoReportEngineMetaData.OPENDOCUMENT_CHART);
        enumerationValues.add(CONTENT_TYPE, PentahoReportEngineMetaData.DEBUG);
    }

    public Object[] getEnumerationValues(final String parameter)
    {
        return enumerationValues.toArray(parameter);
    }

    public Class getParameterType(final String parameter)
    {
        return (Class) parameterTypes.get(parameter);
    }

    public boolean isEnumeration(final String parameter)
    {
        return enumerationValues.containsKey(parameter);
    }

    /**
     * Checks, whether a certain output type is available. Feed the mime-type of the output
     * type in and you'll get a true or false back.
     *
     * @param mimeType
     * @return true, if the output type is supported; false otherwise.
     */
    public boolean isOutputSupported(final String mimeType)
    {
        return enumerationValues.containsValue(CONTENT_TYPE, mimeType.toLowerCase());
    }

    /**
     * Lists all supported output parameters for the given mime-type. This listing can be
     * used to build a generic user interface for configuring a certain output.
     *
     * @param mimeType
     * @return
     */
    public String[] getOutputParameters(final String mimeType)
    {
        return new String[0];
    }

    public boolean isMandatory(final String parameter)
    {
        return mandatoryParameters.contains(parameter);
    }
}
