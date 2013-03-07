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
package org.libreoffice.report.pentaho.model;

import org.libreoffice.report.JobProperties;

import org.jfree.report.JFreeReport;

/**
 * An office document represents the root of the report processing. In
 * OpenOffice reports, this is the only child of the report object.
 *
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
