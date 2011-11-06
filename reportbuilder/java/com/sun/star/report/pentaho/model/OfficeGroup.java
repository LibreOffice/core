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

import com.sun.star.report.OfficeToken;
import com.sun.star.report.pentaho.OfficeNamespaces;

import org.jfree.report.JFreeReportInfo;
import org.jfree.report.expressions.Expression;
import org.jfree.report.structure.Section;

/**
 * An office group is a virtual section that contains the group header, footer
 * and either a detail section or another group.
 *
 * @author Thomas Morgner
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
}
