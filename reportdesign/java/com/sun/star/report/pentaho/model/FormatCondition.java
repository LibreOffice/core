/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: FormatCondition.java,v $
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

import org.jfree.report.expressions.FormulaExpression;

/**
 * The format condition defines, what style-format is applied to an element.
 *
 * @author Thomas Morgner
 * @since 02.03.2007
 */
public class FormatCondition
{
  private final FormulaExpression formula;
  private final String styleName;
  private final boolean enabled;

  public FormatCondition(final FormulaExpression formula,
                         final String styleName,
                         final boolean enabled)
  {
    this.formula = formula;
    this.styleName = styleName;
    this.enabled = enabled;
  }


  public FormulaExpression getFormula()
  {
    return formula;
  }

  public String getStyleName()
  {
    return styleName;
  }

  public boolean isEnabled()
  {
    return enabled;
  }
}
