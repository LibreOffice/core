/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: FontFaceDeclsSection.java,v $
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

import com.sun.star.report.pentaho.OfficeNamespaces;
import java.util.Map;
import org.jfree.report.structure.Element;

/**
 * Todo: Document me!
 *
 * @author Thomas Morgner
 * @since 13.03.2007
 */
public class FontFaceDeclsSection extends Element
{
  private final Map fontFaces;

  public FontFaceDeclsSection()
  {
    fontFaces = new HashMap();
    setType("font-face-decls");
    setNamespace(OfficeNamespaces.OFFICE_NS);
  }

  public void addFontFace(final FontFaceElement style)
  {
    fontFaces.put (style.getStyleName(), style);
  }

  public FontFaceElement getFontFace (final String name)
  {
    return (FontFaceElement) fontFaces.get(name);
  }

  public FontFaceElement[] getAllFontFaces()
  {
    return (FontFaceElement[]) fontFaces.values().toArray
        (new FontFaceElement[fontFaces.size()]);
  }


  public boolean containsFont(final String fontName)
  {
    return fontFaces.containsKey(fontName);
  }
}
