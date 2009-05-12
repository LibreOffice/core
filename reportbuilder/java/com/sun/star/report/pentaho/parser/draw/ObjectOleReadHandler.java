/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ObjectOleReadHandler.java,v $
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
package com.sun.star.report.pentaho.parser.draw;

import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.pentaho.model.ObjectOleElement;
import com.sun.star.report.pentaho.parser.ElementReadHandler;
import org.jfree.report.structure.Element;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

/**
 *
 * @author Ocke Janssen
 */
public class ObjectOleReadHandler extends ElementReadHandler
{

    private final static String RPT_CHART_CLASS_ID = "80243D39-6741-46C5-926E-069164FF87BB";
    private final static String OOO_CHART_CLASS_ID = "12DCAE26-281F-416F-A234-C3086127382E";
    private final ObjectOleElement element;

    public ObjectOleReadHandler(final ObjectOleElement element)
    {
        this.element = element;
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

        final String url = attrs.getValue(OfficeNamespaces.XLINK_NS, "href");
        if (url != null)
        {
            element.setUrl(url);
        }

        String classid = attrs.getValue(OfficeNamespaces.DRAWING_NS, "class-id");
        if (classid != null)
        {
            if ( classid.equalsIgnoreCase(RPT_CHART_CLASS_ID))
                classid = OOO_CHART_CLASS_ID;
            element.setClassId(classid);
        }
    }

    public Element getElement()
    {
        return element;
    }
}
