/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OfficeStylesXmlResourceFactory.java,v $
 * $Revision: 1.3 $
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


package com.sun.star.report.pentaho.parser;

import com.sun.star.report.pentaho.model.OfficeStylesCollection;
import org.jfree.report.JFreeReportBoot;
import org.pentaho.reporting.libraries.base.config.Configuration;
import org.pentaho.reporting.libraries.xmlns.parser.AbstractXmlResourceFactory;

/**
 * A LibLoader resource factory for loading an OfficeStyles-collection. This
 * implementation is meant to parse the 'styles.xml' file.
 *
 * @author Thomas Morgner
 * @since 09.03.2007
 */
public class OfficeStylesXmlResourceFactory extends AbstractXmlResourceFactory
{

  public OfficeStylesXmlResourceFactory()
  {
  }


  protected Configuration getConfiguration()
  {
    return JFreeReportBoot.getInstance().getGlobalConfig();
  }

  public Class getFactoryType()
  {
    return OfficeStylesCollection.class;
  }

}
