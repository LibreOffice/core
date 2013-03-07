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
package org.libreoffice.report.pentaho.parser;

import org.libreoffice.report.pentaho.model.OfficeStylesCollection;

import org.jfree.report.JFreeReportBoot;

import org.pentaho.reporting.libraries.base.config.Configuration;
import org.pentaho.reporting.libraries.xmlns.parser.AbstractXmlResourceFactory;

/**
 * A LibLoader resource factory for loading an OfficeStyles-collection. This
 * implementation is meant to parse the 'styles.xml' file.
 *
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
