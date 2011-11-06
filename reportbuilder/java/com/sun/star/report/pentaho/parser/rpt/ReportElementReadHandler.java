/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


package com.sun.star.report.pentaho.parser.rpt;

import com.sun.star.report.OfficeToken;
import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.pentaho.model.ReportElement;
import com.sun.star.report.pentaho.parser.ElementReadHandler;

import org.jfree.report.structure.Element;

import org.pentaho.reporting.libraries.xmlns.parser.IgnoreAnyChildReadHandler;
import org.pentaho.reporting.libraries.xmlns.parser.XmlReadHandler;

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
