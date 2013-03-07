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
package org.libreoffice.report.util;

import org.libreoffice.report.DataSourceFactory;
import org.libreoffice.report.InputRepository;
import org.libreoffice.report.JobDefinitionException;
import org.libreoffice.report.JobProperties;
import org.libreoffice.report.OutputRepository;
import org.libreoffice.report.ParameterMap;
import org.libreoffice.report.ReportEngineMetaData;
import org.libreoffice.report.ReportEngineParameterNames;
import org.libreoffice.report.ReportJobDefinition;

public class DefaultReportJobDefinition implements ReportJobDefinition
{

    public static final String OUTPUT_TYPE = "output-type";
    private final DefaultParameterMap parameters;
    private final DefaultJobProperties properties;

    public DefaultReportJobDefinition(final ReportEngineMetaData metaData)
    {
        this.parameters = new DefaultParameterMap();
        this.properties = new DefaultJobProperties(metaData);
    }

    public void setDataSourceFactory(final DataSourceFactory dataSourceFactory)
            throws JobDefinitionException
    {
        this.properties.setProperty(ReportEngineParameterNames.INPUT_DATASOURCE_FACTORY, dataSourceFactory);
    }

    public DataSourceFactory getDataSourceFactory()
    {
        return (DataSourceFactory) this.properties.getProperty(ReportEngineParameterNames.INPUT_DATASOURCE_FACTORY);
    }

    public InputRepository getInputRepository()
    {
        return (InputRepository) this.properties.getProperty(ReportEngineParameterNames.INPUT_REPOSITORY);
    }

    public void setInputRepository(final InputRepository inputRepository)
            throws JobDefinitionException
    {
        this.properties.setProperty(ReportEngineParameterNames.INPUT_REPOSITORY, inputRepository);
    }

    public OutputRepository getOutputRepository()
    {
        return (OutputRepository) this.properties.getProperty(ReportEngineParameterNames.OUTPUT_REPOSITORY);
    }

    public void setOutputRepository(final OutputRepository outputRepository)
            throws JobDefinitionException
    {
        this.properties.setProperty(ReportEngineParameterNames.OUTPUT_REPOSITORY, outputRepository);
    }

    /**
     * The parameters of the root report definition. The parameters for the subreports are
     * defined using mappings, it would not make sense to define them here.
     *
     * @return a map container for query parameters.
     */
    public ParameterMap getQueryParameters()
    {
        return parameters;
    }

    public void setReportDefinitionName(final String reportDefinitionName)
            throws JobDefinitionException
    {
        this.properties.setProperty(ReportEngineParameterNames.INPUT_NAME, reportDefinitionName);
    }

    /**
     * The name under which we can load the report definition.
     *
     * @return the report definition's name.
     */
    public String getReportDefinitionName()
    {
        return (String) this.properties.getProperty(ReportEngineParameterNames.INPUT_NAME);
    }

    public JobProperties getProcessingParameters()
    {
        return properties;
    }
}
