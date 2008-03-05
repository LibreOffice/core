/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ReportElementReadHandler.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:44:16 $
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
package com.sun.star.report.pentaho.parser.rpt;

import com.sun.star.report.pentaho.model.ReportElement;
import com.sun.star.report.pentaho.parser.ElementReadHandler;
import com.sun.star.report.pentaho.OfficeNamespaces;
import org.jfree.report.structure.Element;
import org.jfree.xmlns.parser.XmlReadHandler;
import org.jfree.xmlns.parser.IgnoreAnyChildReadHandler;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

public class ReportElementReadHandler extends ElementReadHandler
{

    private ReportElement element;

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
        element.setPrintWhenGroupChanges("true".equals(printWhenGroupChanges));
        final String printRepeatingValues = attrs.getValue(OfficeNamespaces.OOREPORT_NS, "print-repeated-values");
        element.setPrintRepeatedValues(printRepeatingValues == null || "true".equals(printRepeatingValues));
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
