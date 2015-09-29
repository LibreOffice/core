/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package org.libreoffice.report.pentaho.model;

import org.libreoffice.report.pentaho.OfficeNamespaces;

import org.jfree.report.structure.Element;

/**
 * Holds all style-definitions and provides some simplified lookup methods to
 * grab them by their type and name.
 *
 * <p>For now, we are only interested in 'style:style' nodes. Each of these nodes
 * has a style-name and a style-family. Each style declaration can have a parent
 * style, from which properties are inherited.</p>
 *
 * <p>Style names are unique within the family, no matter whether the style is an
 * automatic style, an common style or a master style.</p>
 *
 * <p>The contents of this element are the union of the 'styles.xml' file (if it
 * exists), the font-declarations and auto-styles of the document-content.xml
 * and the styles declared in the main document.</p>
 *
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
        // However, at this moment, we don't have a clue about the default styles
        // at all. Maybe we should add them to make this implementation more robust
        // against invalid documents.
        return null;
    }

    public boolean containsStyle(final String family, final String name)
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
