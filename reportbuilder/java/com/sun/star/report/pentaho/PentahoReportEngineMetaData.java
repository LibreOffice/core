/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PentahoReportEngineMetaData.java,v $
 * $Revision: 1.6 $
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

import com.sun.star.report.ReportJobFactory;
import java.util.HashMap;
import java.util.HashSet;

import com.sun.star.report.DataSourceFactory;
import com.sun.star.report.InputRepository;
import com.sun.star.report.OutputRepository;
import com.sun.star.report.ReportEngineMetaData;
import com.sun.star.report.ReportEngineParameterNames;
import com.sun.star.report.ImageService;
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
