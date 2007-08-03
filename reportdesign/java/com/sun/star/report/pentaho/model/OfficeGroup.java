/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OfficeGroup.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-03 09:50:20 $
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

import com.sun.star.report.pentaho.OfficeNamespaces;
import org.jfree.report.structure.Section;
import org.jfree.report.structure.Element;

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
    return "true".equals
        (getAttribute(OfficeNamespaces.OOREPORT_NS, "start-new-column"));
  }

  public boolean isResetPageNumber ()
  {
    return "true".equals
        (getAttribute(OfficeNamespaces.OOREPORT_NS, "reset-page-number"));
  }

  public OfficeGroupSection getHeader()
  {
    final OfficeGroupInstanceSection instanceSection =
        (OfficeGroupInstanceSection) findFirstChild(OfficeNamespaces.INTERNAL_NS, "group-instance");
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
        (OfficeGroupInstanceSection) findFirstChild(OfficeNamespaces.INTERNAL_NS, "group-instance");
    if (instanceSection == null)
    {
      return null;
    }
    return (OfficeGroupSection) instanceSection.findFirstChild
        (OfficeNamespaces.OOREPORT_NS, "group-footer");

  }
}
