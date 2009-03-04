/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OfficeStylesCollection.java,v $
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
  private final FontFaceDeclsSection fontFaceDecls;
  private final OfficeStyles automaticStyles;
  private final OfficeStyles commonStyles;
  private final OfficeMasterStyles masterStyles;

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
