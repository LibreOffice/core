/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OfficeMasterStyles.java,v $
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

import java.util.HashMap;

import java.util.Map;
import org.jfree.report.structure.Element;
import org.jfree.report.structure.Section;

/**
 * The master-styles section can have either a master-page, handout-master
 * or draw-layer-set. (The latter ones are ignored for the reporting purposes,
 * they are PowerPoint related.)
 *
 * There is no documentation how the system selects a master-page if there is
 * no master-page assigned to the paragraph. However, it seems as if the
 * master-page called 'Standard' is used as initial default.
 *
 * @author Thomas Morgner
 * @since 13.03.2007
 */
public class OfficeMasterStyles extends Element
{
  private final Map masterPages;
  private final Section otherNodes;

  public OfficeMasterStyles()
  {
    masterPages = new HashMap();
    otherNodes = new Section();
  }

  public void addMasterPage (final OfficeMasterPage masterPage)
  {
    if (masterPage == null)
    {
      throw new NullPointerException();
    }
    this.masterPages.put(masterPage.getStyleName(), masterPage);
  }

  public OfficeMasterPage getMasterPage (final String name)
  {
    return (OfficeMasterPage) masterPages.get (name);
  }

  public OfficeMasterPage[] getAllMasterPages()
  {
    return (OfficeMasterPage[]) masterPages.values().toArray
        (new OfficeMasterPage[masterPages.size()]);
  }

  public Section getOtherNodes()
  {
    return otherNodes;
  }
}
