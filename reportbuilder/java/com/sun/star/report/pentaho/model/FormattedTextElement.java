/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: FormattedTextElement.java,v $
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

import org.jfree.report.expressions.FormulaExpression;

/**
 * A formatted text element takes a formula, evaluates it and formats the
 * given element using some arbitary style. (As this is totally undocumented,
 * we leave this out for now. Either we'll insert a field there or we call
 * an UNO-component to do the formatting.
 *
 * @author Thomas Morgner
 * @since 02.03.2007
 */
public class FormattedTextElement extends ReportElement
{
  private FormulaExpression valueExpression;

  public FormattedTextElement()
  {
  }


  public FormulaExpression getValueExpression()
  {
    return valueExpression;
  }

  public void setValueExpression(final FormulaExpression valueExpression)
  {
    this.valueExpression = valueExpression;
  }
}
