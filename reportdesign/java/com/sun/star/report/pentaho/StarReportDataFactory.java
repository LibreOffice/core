/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StarReportDataFactory.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:04 $
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
  private DataSourceFactory backend;

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
      throw new ReportDataFactoryException("Failed to create report data wrapper");
    }
    catch (org.jfree.report.DataSourceException e)
    {
      throw new ReportDataFactoryException("Failed to query data");
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
