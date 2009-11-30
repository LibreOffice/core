/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SubDocumentReadHandler.java,v $
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
package com.sun.star.report.pentaho.parser.rpt;

import com.sun.star.report.pentaho.model.ObjectOleElement;
import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.OfficeToken;
import com.sun.star.report.pentaho.parser.draw.ObjectOleReadHandler;
import com.sun.star.report.pentaho.parser.text.NoCDATATextContentReadHandler;
import org.jfree.report.structure.Section;
import org.pentaho.reporting.libraries.xmlns.parser.XmlReadHandler;
import org.pentaho.reporting.libraries.xmlns.parser.IgnoreAnyChildReadHandler;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

/**
 *
 * @author Ocke Janssen
 */
public class SubDocumentReadHandler extends NoCDATATextContentReadHandler
{

    private final ObjectOleElement element;
    private boolean ignore = false;

    public SubDocumentReadHandler(final ObjectOleElement element)
    {
        this.element = element;
    }

    public SubDocumentReadHandler(final Section section, final ObjectOleElement element)
    {
        super(section);
        this.element = element;
    }

    public SubDocumentReadHandler(final Section section)
    {
        this(section, new ObjectOleElement());
        ignore = true;
    }

    /**
     * Starts parsing.
     *
     * @param attrs the attributes.
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
    protected void startParsing(final Attributes attrs) throws SAXException
    {
        if (!ignore)
        {
            super.startParsing(attrs);
        }
    }

    /**
     * Returns the handler for a child element.
     *
     * @param tagName the tag name.
     * @param atts    the attributes.
     * @return the handler or null, if the tagname is invalid.
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
    protected XmlReadHandler getHandlerForChild(final String uri,
            final String tagName,
            final Attributes atts)
            throws SAXException
    {
        if (OfficeNamespaces.OOREPORT_NS.equals(uri))
        {
            // expect a report control. The control will modifiy the current
            // element (as we do not separate the elements that strictly ..)
            if ("report-control".equals(tagName))
            {
                return new IgnoreAnyChildReadHandler();
            }
            if ("report-element".equals(tagName))
            {
                return new ReportElementReadHandler(element);
            }
            if ("master-detail-fields".equals(tagName))
            {
                return new MasterDetailReadHandler(element);
            }
        }
        if (OfficeNamespaces.DRAWING_NS.equals(uri))
        {
            final XmlReadHandler readHandler;
            if (OfficeToken.OBJECT_OLE.equals(tagName))
            {
                readHandler = new ObjectOleReadHandler(element);
            }
            else if ("frame".equals(tagName))
            {
                readHandler = new SubDocumentReadHandler(new Section(), element);
            }
            else
            {
                readHandler = null;
            }
            if (readHandler != null)
            {
                getChildren().add(readHandler);
                return readHandler;
            }
        }
        return super.getHandlerForChild(uri, tagName, atts);
    }
}
