/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FontFaceDeclsSection.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:06 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *    Copyright 2007 by Pentaho Corporation
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


package com.sun.star.report.pentaho.model;

import java.util.HashMap;

import com.sun.star.report.pentaho.OfficeNamespaces;
import org.jfree.report.structure.Element;

/**
 * Todo: Document me!
 *
 * @author Thomas Morgner
 * @since 13.03.2007
 */
public class FontFaceDeclsSection extends Element
{
  private HashMap fontFaces;

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
