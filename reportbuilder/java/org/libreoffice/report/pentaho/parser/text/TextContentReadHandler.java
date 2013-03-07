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
package org.libreoffice.report.pentaho.parser.text;

import org.jfree.report.structure.Section;
import org.jfree.report.structure.StaticText;

import org.xml.sax.SAXException;

/**
 * This is a generic implementation that accepts all input and adds special
 * handlers for the report-elements.
 *
 */
public class TextContentReadHandler extends NoCDATATextContentReadHandler
{

    public TextContentReadHandler(final Section section, final boolean copyType)
    {
        super(section, copyType);
    }

    public TextContentReadHandler(final Section section)
    {
        super(section);
    }

    public TextContentReadHandler()
    {
    }

    /**
     * This method is called to process the character data between element tags.
     *
     * @param ch     the character buffer.
     * @param start  the start index.
     * @param length the length.
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
    public void characters(final char[] ch, final int start, final int length)
            throws SAXException
    {
        getChildren().add(new StaticText(new String(ch, start, length)));
    }
}
