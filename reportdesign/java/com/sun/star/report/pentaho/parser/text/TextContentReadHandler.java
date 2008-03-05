/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TextContentReadHandler.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:49:03 $
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
