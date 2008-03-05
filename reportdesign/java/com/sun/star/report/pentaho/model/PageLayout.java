/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PageLayout.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:34:16 $
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

import org.jfree.report.structure.Section;
import com.sun.star.report.pentaho.OfficeNamespaces;

/**
 * A page layout describes the physical properties of a page. It is equal to
 * an @page rule in CSS.
 *
 * @author Thomas Morgner
 * @since 13.03.2007
 */
public class PageLayout extends Section
{

  public PageLayout()
  {
    setNamespace(OfficeNamespaces.STYLE_NS);
    setType("page-layout");
  }

  public String getStyleName()
  {
    return (String) getAttribute(OfficeNamespaces.STYLE_NS, "name");
  }

  public void setStyleName(final String name)
  {
    setAttribute(OfficeNamespaces.STYLE_NS, "name", name);
  }

  public Section getHeaderStyle ()
  {
    return (Section) findFirstChild(OfficeNamespaces.STYLE_NS, "header-style");
  }

  public Section getFooterStyle ()
  {
    return (Section) findFirstChild(OfficeNamespaces.STYLE_NS, "footer-style");
  }
}
