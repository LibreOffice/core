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

import org.libreoffice.report.JobDefinitionException;
import org.libreoffice.report.JobProperties;
import org.libreoffice.report.ReportEngineMetaData;

import java.util.HashMap;
import java.util.Map;


public class DefaultJobProperties implements JobProperties
{

    private final ReportEngineMetaData metaData;
    private final Map<String,Object> properties;

    public DefaultJobProperties(final ReportEngineMetaData metaData)
    {
        if (metaData == null)
        {
            throw new NullPointerException();
        }
        this.properties = new HashMap<String,Object>();
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
