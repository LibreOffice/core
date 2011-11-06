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


package com.sun.star.report.util;

import com.sun.star.report.JobDefinitionException;
import com.sun.star.report.JobProperties;
import com.sun.star.report.ReportEngineMetaData;

import java.util.HashMap;
import java.util.Map;


public class DefaultJobProperties implements JobProperties
{

    private final ReportEngineMetaData metaData;
    private final Map properties;

    public DefaultJobProperties(final ReportEngineMetaData metaData)
    {
        if (metaData == null)
        {
            throw new NullPointerException();
        }
        this.properties = new HashMap();
        this.metaData = metaData;
    }

    public Object getProperty(final String key)
    {
        return properties.get(key);
    }

    public void setProperty(final String key, final Object value)
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
