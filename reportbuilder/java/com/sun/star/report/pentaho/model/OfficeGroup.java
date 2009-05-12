/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OfficeGroup.java,v $
 * $Revision: 1.6 $
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


package com.sun.star.report.pentaho.model;

import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.OfficeToken;
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

  public boolean isStartNewColumn ()
  {
    return OfficeToken.TRUE.equals
        (getAttribute(OfficeNamespaces.OOREPORT_NS, "start-new-column"));
  }

  public boolean isResetPageNumber ()
  {
    return OfficeToken.TRUE.equals
        (getAttribute(OfficeNamespaces.OOREPORT_NS, "reset-page-number"));
  }

  public OfficeGroupSection getHeader()
  {
    final OfficeGroupInstanceSection instanceSection =
        (OfficeGroupInstanceSection) findFirstChild(JFreeReportInfo.REPORT_NAMESPACE, "group-instance");
    if (instanceSection == null)
    {
      return null;
    }
    return (OfficeGroupSection) instanceSection.findFirstChild
        (OfficeNamespaces.OOREPORT_NS, "group-header");

  }

  public OfficeGroupSection getFooter()
  {
    final OfficeGroupInstanceSection instanceSection =
        (OfficeGroupInstanceSection) findFirstChild(JFreeReportInfo.REPORT_NAMESPACE, "group-instance");
    if (instanceSection == null)
    {
      return null;
    }
    return (OfficeGroupSection) instanceSection.findFirstChild
        (OfficeNamespaces.OOREPORT_NS, "group-footer");

  }
  public Expression getGroupingExpression(){
      final OfficeGroupInstanceSection instanceSection =
        (OfficeGroupInstanceSection) findFirstChild(JFreeReportInfo.REPORT_NAMESPACE, "group-instance");
    if (instanceSection == null)
    {
      return null;
    }
    return instanceSection.getGroupingExpression();
  }
}
