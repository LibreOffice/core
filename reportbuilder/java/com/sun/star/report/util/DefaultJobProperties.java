/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DefaultJobProperties.java,v $
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

import java.util.HashMap;

import com.sun.star.report.JobProperties;
import com.sun.star.report.ReportEngineMetaData;
import com.sun.star.report.JobDefinitionException;
import java.util.Map;

public class DefaultJobProperties implements JobProperties
{
  private final ReportEngineMetaData metaData;
  private final Map properties;

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
    if (!type.isInstance(value))
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
