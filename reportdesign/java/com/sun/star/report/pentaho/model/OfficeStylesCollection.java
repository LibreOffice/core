/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OfficeStylesCollection.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:07 $
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

import com.sun.star.report.pentaho.OfficeNamespaces;
import org.jfree.report.structure.Element;

/**
 * Holds all style-definitions and provides some simplified lookup methods to
 * grab them by their type and name.
 * <p/>
 * For now, we are only interested in 'style:style' nodes. Each of these nodes
 * has a style-name and a style-family. Each style declaration can have a parent
 * style, from which properties are inherited.
 * <p/>
 * Style names are unique within the family, no matter whether the style is an
 * automatic style, an common style or a master style.
 * <p/>
 * The contents of this element are the union of the 'styles.xml' file (if it
 * exists), the font-declarations and auto-styles of the document-content.xml
 * and the styles declared in the main document.
 *
 * @author Thomas Morgner
 * @since 06.03.2007
 */
public class OfficeStylesCollection extends Element
{
  // Font-face declarations are copied as is. We simply merge them by adding
  // them all in one set. This may result in duplicate entries, but as the
  // fileformat does not forbid that, it therefore must be ok.
  private FontFaceDeclsSection fontFaceDecls;
  private OfficeStyles automaticStyles;
  private OfficeStyles commonStyles;
  private OfficeMasterStyles masterStyles;

  public OfficeStylesCollection()
  {
    fontFaceDecls = new FontFaceDeclsSection();

    automaticStyles = new OfficeStyles();
    automaticStyles.setType("automatic-styles");
    automaticStyles.setNamespace(OfficeNamespaces.OFFICE_NS);

    commonStyles = new OfficeStyles();
    commonStyles.setType("styles");
    commonStyles.setNamespace(OfficeNamespaces.OFFICE_NS);

    masterStyles = new OfficeMasterStyles();
    masterStyles.setType("master-styles");
    masterStyles.setNamespace(OfficeNamespaces.OFFICE_NS);
  }

  public OfficeStyle getStyle(final String family, final String name)
  {
    final OfficeStyle commonStyle = commonStyles.getStyle(family, name);
    if (commonStyle != null)
    {
      return commonStyle;
    }
    final OfficeStyle autoStyle = automaticStyles.getStyle(family, name);
    if (autoStyle != null)
    {
      return autoStyle;
    }

    // And later: Autogenerate one of the default styles.
    // However, at this moment, we dont have a clue about the default styles
    // at all. Maybe we should add them to make this implementation more robust
    // against invalid documents.
    return null;
  }

  public boolean containsStyle (final String family, final String name)
  {
    return (getStyle(family, name) != null);
  }

  public OfficeStyles getAutomaticStyles()
  {
    return automaticStyles;
  }

  public OfficeStyles getCommonStyles()
  {
    return commonStyles;
  }

  public OfficeMasterStyles getMasterStyles()
  {
    return masterStyles;
  }

  public FontFaceDeclsSection getFontFaceDecls()
  {
    return fontFaceDecls;
  }
}
