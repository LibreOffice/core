/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ImageElement.java,v $
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
import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.OfficeToken;

/**
 * Todo: Document me!
 *
 * @author Thomas Morgner
 * @since 02.03.2007
 */
public class ImageElement extends ReportElement
{
  private FormulaExpression formula;

  public ImageElement()
  {
  }

  public FormulaExpression getFormula()
  {
    return formula;
  }

  public void setFormula(final FormulaExpression formula)
  {
    this.formula = formula;
  }


    public String getScaleMode()
    {
        String val = (String)getAttribute(OfficeNamespaces.OOREPORT_NS, OfficeToken.SCALE);
        if ( OfficeToken.TRUE.equals(val) )
            val = OfficeToken.ANISOTROPIC;
        else if ( OfficeToken.FALSE.equals(val) || val == null )
            val = OfficeToken.NONE;
        return val;
    }

  public boolean isPreserveIRI()
  {
    return OfficeToken.TRUE.equals(getAttribute(OfficeNamespaces.OOREPORT_NS, OfficeToken.PRESERVE_IRI));
  }

  public void setPreserveIRI(final boolean preserveIRI)
  {
    setAttribute(OfficeNamespaces.OOREPORT_NS, OfficeToken.PRESERVE_IRI, String.valueOf(preserveIRI));
  }

  public String getImageData()
  {
    return (String) getAttribute(OfficeNamespaces.FORM_NS, OfficeToken.IMAGE_DATA);
  }
}
