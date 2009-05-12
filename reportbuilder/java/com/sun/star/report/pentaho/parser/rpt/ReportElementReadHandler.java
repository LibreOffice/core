/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ReportElementReadHandler.java,v $
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
package com.sun.star.report.pentaho.parser.rpt;

import com.sun.star.report.pentaho.model.ReportElement;
import com.sun.star.report.pentaho.parser.ElementReadHandler;
import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.OfficeToken;
import org.jfree.report.structure.Element;
import org.pentaho.reporting.libraries.xmlns.parser.XmlReadHandler;
import org.pentaho.reporting.libraries.xmlns.parser.IgnoreAnyChildReadHandler;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

public class ReportElementReadHandler extends ElementReadHandler
{

    private final ReportElement element;

    public ReportElementReadHandler(final ReportElement element)
    {
        if (element == null)
        {
            throw new NullPointerException();
        }

        this.element = element;
    }

    public Element getElement()
    {
        return element;
    }

    /**
     * Starts parsing.
     *
     * @param attrs the attributes.
     * @throws org.xml.sax.SAXException if there is a parsing error.
     */
    protected void startParsing(final Attributes attrs)
            throws SAXException
    {
        super.startParsing(attrs);
        final String printWhenGroupChanges = attrs.getValue(OfficeNamespaces.OOREPORT_NS, "print-when-group-changes");
        element.setPrintWhenGroupChanges(OfficeToken.TRUE.equals(printWhenGroupChanges));
        final String printRepeatingValues = attrs.getValue(OfficeNamespaces.OOREPORT_NS, "print-repeated-values");
        element.setPrintRepeatedValues(printRepeatingValues == null || OfficeToken.TRUE.equals(printRepeatingValues));
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
        if (!OfficeNamespaces.OOREPORT_NS.equals(uri))
        {
            return null;
        }
        if ("conditional-print-expression".equals(tagName))
        {
            return new ConditionalPrintExpressionReadHandler(element);
        }
        if ("format-condition".equals(tagName))
        {
            return new FormatConditionReadHandler(element);
        }
        if ("report-component".equals(tagName))
        {
            return new IgnoreAnyChildReadHandler();
        }
        return null;
    }
}
