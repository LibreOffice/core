/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: VariablesDeclarationSection.java,v $
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


package com.sun.star.report.pentaho.model;

import org.jfree.report.JFreeReportInfo;
import org.jfree.report.structure.Section;

/**
 * A paragraph that contains variables-declarations. This paragraph will be
 * printed in the first cell of the first table (if there's one).
 *
 * The VariablesDeclarationSection is an auto-generated structure element that
 * has no model-representation. The section itself is empty and simply acts
 * as flag for the output-processor. The output processor itself is responsible
 * to maintain the variables.
 *
 * @author Thomas Morgner
 * @since 19.03.2007
 */
public class VariablesDeclarationSection extends Section
{

  public VariablesDeclarationSection()
  {
    setNamespace(JFreeReportInfo.REPORT_NAMESPACE);
    setType("variables-section");
  }
}
