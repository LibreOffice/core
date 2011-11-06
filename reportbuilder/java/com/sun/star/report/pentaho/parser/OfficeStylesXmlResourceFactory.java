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


package com.sun.star.report.pentaho.parser;

import com.sun.star.report.pentaho.model.OfficeStylesCollection;

import org.jfree.report.JFreeReportBoot;

import org.pentaho.reporting.libraries.base.config.Configuration;
import org.pentaho.reporting.libraries.xmlns.parser.AbstractXmlResourceFactory;

/**
 * A LibLoader resource factory for loading an OfficeStyles-collection. This
 * implementation is meant to parse the 'styles.xml' file.
 *
 * @author Thomas Morgner
 * @since 09.03.2007
 */
public class OfficeStylesXmlResourceFactory extends AbstractXmlResourceFactory
{

    public OfficeStylesXmlResourceFactory()
    {
    }

    protected Configuration getConfiguration()
    {
        return JFreeReportBoot.getInstance().getGlobalConfig();
    }

    public Class getFactoryType()
    {
        return OfficeStylesCollection.class;
    }
}
