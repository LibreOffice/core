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


package com.sun.star.report.pentaho.model;

import com.sun.star.report.JobProperties;

import org.jfree.report.JFreeReport;

/**
 * An office document represents the root of the report processing. In
 * OpenOffice reports, this is the only child of the report object.
 *
 * @author Thomas Morgner
 * @since 02.03.2007
 */
public class OfficeDocument extends JFreeReport
{

    private OfficeStylesCollection stylesCollection;
    private JobProperties jobProperties;

    public JobProperties getJobProperties()
    {
        return jobProperties;
    }

    public void setJobProperties(final JobProperties jobProperties)
    {
        this.jobProperties = jobProperties;
    }

    public OfficeDocument()
    {
    }

    public OfficeStylesCollection getStylesCollection()
    {
        return stylesCollection;
    }

    public void setStylesCollection(final OfficeStylesCollection stylesCollection)
    {
        if (stylesCollection == null)
        {
            throw new NullPointerException();
        }
        this.stylesCollection = stylesCollection;
    }
}
