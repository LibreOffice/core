/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DefaultReportJobDefinition.java,v $
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


package com.sun.star.report.util;

import com.sun.star.report.DataSourceFactory;
import com.sun.star.report.InputRepository;
import com.sun.star.report.JobDefinitionException;
import com.sun.star.report.JobProperties;
import com.sun.star.report.OutputRepository;
import com.sun.star.report.ParameterMap;
import com.sun.star.report.ReportEngineMetaData;
import com.sun.star.report.ReportEngineParameterNames;
import com.sun.star.report.ReportJobDefinition;

public class DefaultReportJobDefinition implements ReportJobDefinition
{
  public static final String OUTPUT_TYPE = "output-type";

  private final DefaultParameterMap parameters;
  private final DefaultJobProperties properties;

  public DefaultReportJobDefinition (final ReportEngineMetaData metaData)
  {
    this.parameters = new DefaultParameterMap();
    this.properties = new DefaultJobProperties(metaData);
  }

  public void setDataSourceFactory (final DataSourceFactory dataSourceFactory)
          throws JobDefinitionException
  {
    this.properties.setProperty(ReportEngineParameterNames.INPUT_DATASOURCE_FACTORY, dataSourceFactory);
  }

  public DataSourceFactory getDataSourceFactory ()
  {
    return (DataSourceFactory) this.properties.getProperty(ReportEngineParameterNames.INPUT_DATASOURCE_FACTORY);
  }

  public InputRepository getInputRepository ()
  {
    return (InputRepository)
            this.properties.getProperty(ReportEngineParameterNames.INPUT_REPOSITORY);
  }

  public void setInputRepository (final InputRepository inputRepository)
          throws JobDefinitionException
  {
    this.properties.setProperty(ReportEngineParameterNames.INPUT_REPOSITORY, inputRepository);
  }

  public OutputRepository getOutputRepository ()
  {
    return (OutputRepository)
            this.properties.getProperty(ReportEngineParameterNames.OUTPUT_REPOSITORY);
  }

  public void setOutputRepository (final OutputRepository outputRepository)
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
  public ParameterMap getQueryParameters ()
  {
    return parameters;
  }

  public void setReportDefinitionName (final String reportDefinitionName)
          throws JobDefinitionException
  {
    this.properties.setProperty(ReportEngineParameterNames.INPUT_NAME, reportDefinitionName);
  }

  /**
   * The name under which we can load the report definition.
   *
   * @return the report definition's name.
   */
  public String getReportDefinitionName ()
  {
    return (String) this.properties.getProperty(ReportEngineParameterNames.INPUT_NAME);
  }

  public JobProperties getProcessingParameters ()
  {
    return properties;
  }
}
