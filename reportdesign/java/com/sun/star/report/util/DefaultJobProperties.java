/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DefaultJobProperties.java,v $
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

import java.util.HashMap;

import com.sun.star.report.JobProperties;
import com.sun.star.report.ReportEngineMetaData;
import com.sun.star.report.JobDefinitionException;

public class DefaultJobProperties implements JobProperties
{
  private ReportEngineMetaData metaData;
  private HashMap properties;

  public DefaultJobProperties (final ReportEngineMetaData metaData)
  {
    if (metaData == null)
    {
      throw new NullPointerException();
    }
    this.properties = new HashMap();
    this.metaData = metaData;
  }

  public Object getProperty (final String key)
  {
    return properties.get(key);
  }

  public void setProperty (final String key, final Object value)
          throws JobDefinitionException
  {
    final Class type = metaData.getParameterType(key);
    if (type == null)
    {
      throw new JobDefinitionException("The parameter name is not known: " + key);
    }
    if (type.isInstance(value) == false)
    {
      throw new JobDefinitionException("The parameter value is not understood");
    }

    this.properties.put(key, value);
  }

  public JobProperties copy()
  {
    final DefaultJobProperties props = new DefaultJobProperties(metaData);
    props.properties.putAll(properties);
    return props;
  }
}
