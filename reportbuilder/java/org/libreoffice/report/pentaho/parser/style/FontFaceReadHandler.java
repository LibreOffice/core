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
package org.libreoffice.report.pentaho.parser.style;

import org.libreoffice.report.pentaho.model.FontFaceElement;
import org.libreoffice.report.pentaho.parser.ElementReadHandler;

import org.jfree.report.structure.Element;

/**
 * Reads a single font-face declaration. This is equal to the @font rule of
 * the CSS standard. We do not interpret the attributes of the element in
 * any way yet.
 *
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
