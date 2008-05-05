/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: FontFaceReadHandler.java,v $
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


package com.sun.star.report.pentaho.parser.style;

import com.sun.star.report.pentaho.parser.ElementReadHandler;
import com.sun.star.report.pentaho.model.FontFaceElement;
import org.jfree.report.structure.Element;

/**
 * Reads a single font-face declaration. This is equal to the @font rule of
 * the CSS standard. We do not interpret the attributes of the element in
 * any way yet.
 *
 * @author Thomas Morgner
 * @since 13.03.2007
 */
public class FontFaceReadHandler extends ElementReadHandler
{
  private final FontFaceElement fontFaceElement;

  public FontFaceReadHandler()
  {
    this.fontFaceElement = new FontFaceElement();
  }

  public Element getElement()
  {
    return fontFaceElement;
  }
}
