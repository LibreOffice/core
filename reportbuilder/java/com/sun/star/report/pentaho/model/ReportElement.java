/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ReportElement.java,v $
 * $Revision: 1.4 $
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

import java.util.ArrayList;

import org.jfree.report.structure.Element;
import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.OfficeToken;
import java.util.List;

/**
 * A report element is the base class for all content generating elements in a
 * report.
 *
 * @author Thomas Morgner
 * @since 02.03.2007
 */
public abstract class ReportElement extends Element
{
  private final List formatConditions;

  protected ReportElement()
  {
    formatConditions = new ArrayList();
  }

  /**
   * Checks the current group and prints this element only if the current row is
   * the first row for that particular group.
   *
   * @return true, if the element should only be printed in the first row of the
   *         current group, false otherwise.
   */
  public boolean isPrintWhenGroupChanges()
  {
    return OfficeToken.TRUE.equals(getAttribute
        (OfficeNamespaces.OOREPORT_NS, "print-when-group-changes"));
  }

  public void setPrintWhenGroupChanges(final boolean printWhenGroupChanges)
  {
    setAttribute(OfficeNamespaces.OOREPORT_NS, "print-when-group-changes",
        String.valueOf(printWhenGroupChanges));
  }

  /**
   * Checks, whether the printed value has been changed since the last run. The
   * element will only be printed, if there was at least one change.
   *
   * @return true, if repeated values should be printed, false if repeated
   *         values should be surpressed.
   */
  public boolean isPrintRepeatedValues()
  {
    return OfficeToken.TRUE.equals(getAttribute
        (OfficeNamespaces.OOREPORT_NS, "print-repeated-values"));
  }

  public void setPrintRepeatedValues(final boolean printRepeatedValues)
  {
    setAttribute(OfficeNamespaces.OOREPORT_NS, "print-repeated-values",
        String.valueOf(printRepeatedValues));
  }

  public void addFormatCondition(final FormatCondition formatCondition)
  {
    if (formatCondition == null)
    {
      throw new NullPointerException();
    }

    this.formatConditions.add(formatCondition);
  }

  public FormatCondition[] getFormatConditions ()
  {
    return (FormatCondition[]) this.formatConditions.toArray
        (new FormatCondition[this.formatConditions.size()]);
  }

  public int getFormatConditionCount ()
  {
    return formatConditions.size();
  }
}
