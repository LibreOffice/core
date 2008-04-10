/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PageLayout.java,v $
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
