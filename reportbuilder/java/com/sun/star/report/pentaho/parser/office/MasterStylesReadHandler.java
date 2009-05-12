/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: MasterStylesReadHandler.java,v $
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
package com.sun.star.report.pentaho.parser.office;

import java.util.ArrayList;

import com.sun.star.report.pentaho.parser.ElementReadHandler;
import com.sun.star.report.pentaho.parser.style.StyleDefinitionReadHandler;
import com.sun.star.report.pentaho.parser.style.MasterPageReadHandler;
import com.sun.star.report.pentaho.model.OfficeMasterStyles;
import com.sun.star.report.pentaho.OfficeNamespaces;
import java.util.List;
import org.jfree.report.structure.Element;
import org.pentaho.reporting.libraries.xmlns.parser.XmlReadHandler;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

/**
 * Todo: Document me!
 *
 * @author Thomas Morgner
 * @since 13.03.2007
 */
public class MasterStylesReadHandler extends ElementReadHandler
{

    private final OfficeMasterStyles masterStyles;
    private final List otherHandlers;
    private final List masterPageHandlers;

    public MasterStylesReadHandler(final OfficeMasterStyles masterStyles)
    {
        this.masterStyles = masterStyles;
        this.masterPageHandlers = new ArrayList();
        this.otherHandlers = new ArrayList();
    }

    public OfficeMasterStyles getMasterStyles()
    {
        return masterStyles;
    }

    /**
     * Returns the handler for a child element.
     *
     * @param tagName the tag name.
     * @param atts    the attributes.
     * @return the handler or null, if the tagname is invalid.
     *
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
    protected XmlReadHandler getHandlerForChild(final String uri,
            final String tagName,
            final Attributes atts)
            throws SAXException
    {
        if (OfficeNamespaces.STYLE_NS.equals(uri) && "master-page".equals(tagName))
        {
            final MasterPageReadHandler mrh = new MasterPageReadHandler();
            masterPageHandlers.add(mrh);
            return mrh;
        }

        final StyleDefinitionReadHandler readHandler =
                new StyleDefinitionReadHandler();
        otherHandlers.add(readHandler);
        return readHandler;
    }

    /**
     * Done parsing.
     *
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
    protected void doneParsing()
            throws SAXException
    {
        for (int i = 0; i < otherHandlers.size(); i++)
        {
            final ElementReadHandler handler =
                    (ElementReadHandler) otherHandlers.get(i);
            masterStyles.getOtherNodes().addNode(handler.getElement());
        }

        for (int i = 0; i < masterPageHandlers.size(); i++)
        {
            final MasterPageReadHandler handler =
                    (MasterPageReadHandler) masterPageHandlers.get(i);
            masterStyles.addMasterPage(handler.getMasterPage());
        }
    }

    public Element getElement()
    {
        return masterStyles;
    }
}
