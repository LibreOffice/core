/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: MasterDetailReadHandler.java,v $
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
import com.sun.star.report.pentaho.parser.ElementReadHandler;
import com.sun.star.report.pentaho.OfficeNamespaces;
import org.jfree.report.structure.Element;
import org.pentaho.reporting.libraries.xmlns.parser.XmlReadHandler;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

/**
 *
 * @author Ocke Janssen
 */
public class MasterDetailReadHandler extends ElementReadHandler
{

    private final ObjectOleElement element;
    private final boolean parseMasterDetail;

    public MasterDetailReadHandler(final ObjectOleElement element)
    {
        this.element = element;
        parseMasterDetail = false;
    }

    public MasterDetailReadHandler(final ObjectOleElement element, final boolean parseMasterDetail)
    {
        this.element = element;
        this.parseMasterDetail = parseMasterDetail;
    }

    /**
     * Starts parsing.
     *
     * @param attrs the attributes.
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
    protected void startParsing(final Attributes attrs) throws SAXException
    {
        super.startParsing(attrs);
        if (parseMasterDetail)
        {
            final String master = attrs.getValue(OfficeNamespaces.OOREPORT_NS, "master");
            if (master != null && master.length() > 0 )
            {
                final String detail = attrs.getValue(OfficeNamespaces.OOREPORT_NS, "detail");
                element.addMasterDetailFields(master, detail);
            }
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
        if (OfficeNamespaces.OOREPORT_NS.equals(uri) && "master-detail-field".equals(tagName))
        {
            // expect a report control. The control will modifiy the current
            // element (as we do not separate the elements that strictly ..)
            return new MasterDetailReadHandler(element, true);
        }

        return null;
    }

    public Element getElement()
    {
        return element;
    }
}
