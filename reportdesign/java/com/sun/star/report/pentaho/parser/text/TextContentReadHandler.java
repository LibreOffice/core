/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: TextContentReadHandler.java,v $
 * $Revision: 1.5 $
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
package com.sun.star.report.pentaho.parser.text;

import org.jfree.report.structure.Section;
import org.jfree.report.structure.StaticText;
import org.xml.sax.SAXException;

/**
 * This is a generic implementation that accepts all input and adds special
 * handlers for the report-elements.
 *
 * @author Thomas Morgner
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
