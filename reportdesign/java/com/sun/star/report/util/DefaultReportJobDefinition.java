/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DefaultReportJobDefinition.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:13 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *    Copyright 2007 by Pentaho Corporation
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

  private DefaultParameterMap parameters;
  private DefaultJobProperties properties;

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
