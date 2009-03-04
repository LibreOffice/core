/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: XLinkReadHandler.java,v $
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
package com.sun.star.report.pentaho.parser.xlink;

import com.sun.star.report.pentaho.OfficeNamespaces;
import org.pentaho.reporting.libraries.xmlns.parser.AbstractXmlReadHandler;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

/**
 * An image reference.
 *
 * @author Thomas Morgner
 */
public class XLinkReadHandler extends AbstractXmlReadHandler
{

    private String uri;
    private String type;
    private String show;
    private String actuate;

    public XLinkReadHandler()
    {
    }

    /**
     * Starts parsing.
     *
     * @param attrs the attributes.
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
    protected void startParsing(final Attributes attrs) throws SAXException
    {
        uri = attrs.getValue(OfficeNamespaces.XLINK_NS, "uri");
        type = attrs.getValue(OfficeNamespaces.XLINK_NS, "type");
        show = attrs.getValue(OfficeNamespaces.XLINK_NS, "show");
        actuate = attrs.getValue(OfficeNamespaces.XLINK_NS, "actuate");
    }

    /**
     * Returns the object for this element or null, if this element does not
     * create an object.
     *
     * @return the object.
     */
    public Object getObject() throws SAXException
    {
        return uri;
    }

    public String getUri()
    {
        return uri;
    }

    public String getType()
    {
        return type;
    }

    public String getShow()
    {
        return show;
    }

    public String getActuate()
    {
        return actuate;
    }
}
