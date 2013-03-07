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

import java.util.HashMap;
import java.util.Map;

import org.jfree.report.structure.Element;


/**
 * Todo: Document me!
 *
 * @since 13.03.2007
 */
public class FontFaceDeclsSection extends Element
{

    private final Map<String,FontFaceElement> fontFaces;

    public FontFaceDeclsSection()
    {
        fontFaces = new HashMap<String,FontFaceElement>();
        setType("font-face-decls");
        setNamespace(OfficeNamespaces.OFFICE_NS);
    }

    public void addFontFace(final FontFaceElement style)
    {
        fontFaces.put(style.getStyleName(), style);
    }

    public FontFaceElement getFontFace(final String name)
    {
        return fontFaces.get(name);
    }

    public FontFaceElement[] getAllFontFaces()
    {
        return fontFaces.values().toArray(new FontFaceElement[fontFaces.size()]);
    }

    public boolean containsFont(final String fontName)
    {
        return fontFaces.containsKey(fontName);
    }
}
