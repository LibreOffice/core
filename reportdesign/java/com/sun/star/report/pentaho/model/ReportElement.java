/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ReportElement.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:07 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *    Copyright 2007 by Pentaho Corporation
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


package com.sun.star.report.pentaho.model;

import java.util.ArrayList;

import org.jfree.report.structure.Element;
import com.sun.star.report.pentaho.OfficeNamespaces;

/**
 * A report element is the base class for all content generating elements in a
 * report.
 *
 * @author Thomas Morgner
 * @since 02.03.2007
 */
public abstract class ReportElement extends Element
{
  private ArrayList formatConditions;

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
    return "true".equals(getAttribute
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
    return "true".equals(getAttribute
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
