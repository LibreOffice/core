/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SDBCReportDataFactory.java,v $
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
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
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
package com.sun.star.report;


import java.util.Map;

import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XRowSet;

/**
 * Very primitive implementation, just to show how this could be used ...
 *
 */
public class SDBCReportDataFactory implements DataSourceFactory
{
  private XRowSet rowSet;

  public SDBCReportDataFactory(final XRowSet rowSet)
  {
    this.rowSet = rowSet;
  }

    public DataSource queryData(final String query, final Map parameters) throws DataSourceException {
        try
        {
          return new SDBCReportData(rowSet);
        }
        catch (SQLException e)
        {
          throw new DataSourceException(e.getMessage(),e);
        }
    }
}
