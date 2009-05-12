/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: StarReportDataFactory.java,v $
 * $Revision: 1.5 $
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

import java.util.HashMap;

import org.jfree.report.ReportDataFactory;
import org.jfree.report.ReportData;
import org.jfree.report.DataSet;
import org.jfree.report.ReportDataFactoryException;
import com.sun.star.report.DataSourceException;
import com.sun.star.report.DataSourceFactory;

public class StarReportDataFactory implements ReportDataFactory, Cloneable
{
  private final DataSourceFactory backend;

  public StarReportDataFactory (DataSourceFactory backend)
  {
    this.backend = backend;
  }

  /**
   * Queries a datasource. The string 'query' defines the name of the query. The
   * Parameterset given here may contain more data than actually needed.
   * <p/>
   * The dataset may change between two calls, do not assume anything!
   *
   * @param query
   * @param parameters
   * @return
   */
  public ReportData queryData (final String query, final DataSet parameters)
          throws ReportDataFactoryException
  {
    try
    {
      final HashMap map = new HashMap();
      final int count = parameters.getColumnCount();
      for (int i = 0; i < count; i++)
      {
        final Object o = parameters.get(i);
        map.put (parameters.getColumnName(i), o);
      }
      return new StarReportData(backend.queryData(query, map));
    }
    catch(DataSourceException dse)
    {
        String message = dse.getMessage();
        if ( message.length() == 0 )
            message = "Failed to create report data wrapper";
      throw new ReportDataFactoryException(message,dse);
    }
    catch (org.jfree.report.DataSourceException e)
    {
        String message = e.getMessage();
        if ( message.length() == 0 )
            message = "Failed to query data";
      throw new ReportDataFactoryException(message);
    }
  }

  public void open()
  {

  }

  public void close()
  {

  }


  /**
   * Derives a freshly initialized report data factory, which is independend of
   * the original data factory. Opening or Closing one data factory must not
   * affect the other factories.
   *
   * @return
   */
  public ReportDataFactory derive()
  {
    try
    {
      return (ReportDataFactory) clone();
    }
    catch (CloneNotSupportedException e)
    {
      throw new IllegalStateException("Clone failed?");
    }
  }

  public Object clone () throws CloneNotSupportedException
  {
    return super.clone();
  }
}
