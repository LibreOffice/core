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
package org.libreoffice.report.pentaho;

import org.libreoffice.report.ReportAddIn;
import org.libreoffice.report.ReportExpression;
import org.libreoffice.report.ReportExpressionMetaData;
import org.libreoffice.report.pentaho.expressions.SumExpression;

/**
 * This class is a dummy implementation. Ignore it for now, we may extend this
 * one later.
 */
public class PentahoReportAddIn implements ReportAddIn
{

    public PentahoReportAddIn()
    {
    }

    public ReportExpression createExpression(final int expression)
    {
        return (expression == 0) ? new SumExpression() : null;
    }

    public int getExpressionCount()
    {
        return 1;
    }

    public ReportExpressionMetaData getMetaData(final int expression)
    {
        // todo implement me
        return null;
    }
}
