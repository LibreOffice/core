/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OfficeDocument.java,v $
 * $Revision: 1.5 $
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

import com.sun.star.report.JobProperties;
import org.jfree.report.JFreeReport;

/**
 * An office document represents the root of the report processing. In
 * OpenOffice reports, this is the only child of the report object.
 *
 * @author Thomas Morgner
 * @since 02.03.2007
 */
public class OfficeDocument extends JFreeReport
{
  private OfficeStylesCollection stylesCollection;
  private JobProperties jobProperties;

    public JobProperties getJobProperties() {
        return jobProperties;
    }

    public void setJobProperties(final JobProperties jobProperties) {
        this.jobProperties = jobProperties;
    }

  public OfficeDocument()
  {
  }

  public OfficeStylesCollection getStylesCollection()
  {
    return stylesCollection;
  }

  public void setStylesCollection(final OfficeStylesCollection stylesCollection)
  {
    if (stylesCollection == null)
    {
      throw new NullPointerException();
    }
    this.stylesCollection = stylesCollection;
  }
}
