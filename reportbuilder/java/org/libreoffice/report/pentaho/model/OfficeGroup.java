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

import org.libreoffice.report.OfficeToken;
import org.libreoffice.report.pentaho.OfficeNamespaces;

import org.jfree.report.JFreeReportInfo;
import org.jfree.report.expressions.Expression;
import org.jfree.report.structure.Section;

/**
 * An office group is a virtual section that contains the group header, footer
 * and either a detail section or another group.
 *
 * @since 02.03.2007
 */
public class OfficeGroup extends Section
{

    public OfficeGroup()
    {
    }

    public boolean isStartNewColumn()
    {
        return OfficeToken.TRUE.equals(getAttribute(OfficeNamespaces.OOREPORT_NS, "start-new-column"));
    }

    public boolean isResetPageNumber()
    {
        return OfficeToken.TRUE.equals(getAttribute(OfficeNamespaces.OOREPORT_NS, "reset-page-number"));
    }

    public OfficeGroupSection getHeader()
    {
        final OfficeGroupInstanceSection instanceSection =
                (OfficeGroupInstanceSection) findFirstChild(JFreeReportInfo.REPORT_NAMESPACE, "group-instance");
        if (instanceSection == null)
        {
            return null;
        }
        return (OfficeGroupSection) instanceSection.findFirstChild(OfficeNamespaces.OOREPORT_NS, "group-header");

    }

    public OfficeGroupSection getFooter()
    {
        final OfficeGroupInstanceSection instanceSection =
                (OfficeGroupInstanceSection) findFirstChild(JFreeReportInfo.REPORT_NAMESPACE, "group-instance");
        if (instanceSection == null)
        {
            return null;
        }
        return (OfficeGroupSection) instanceSection.findFirstChild(OfficeNamespaces.OOREPORT_NS, "group-footer");

    }

    public Expression getGroupingExpression()
    {
        final OfficeGroupInstanceSection instanceSection =
                (OfficeGroupInstanceSection) findFirstChild(JFreeReportInfo.REPORT_NAMESPACE, "group-instance");
        if (instanceSection == null)
        {
            return null;
        }
        return instanceSection.getGroupingExpression();
    }

    public String getSortingExpression()
    {
        final OfficeGroupInstanceSection instanceSection =
                (OfficeGroupInstanceSection) findFirstChild(JFreeReportInfo.REPORT_NAMESPACE, "group-instance");
        if (instanceSection == null)
        {
            return null;
        }
        return instanceSection.getSortingExpression();
    }
}
