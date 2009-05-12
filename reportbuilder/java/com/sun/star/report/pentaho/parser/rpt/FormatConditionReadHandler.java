/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: FormatConditionReadHandler.java,v $
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

import com.sun.star.report.pentaho.model.FormatCondition;
import com.sun.star.report.pentaho.model.ReportElement;
import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.OfficeToken;
import org.jfree.report.expressions.FormulaExpression;
import org.pentaho.reporting.libraries.xmlns.parser.AbstractXmlReadHandler;
import org.pentaho.reporting.libraries.xmlns.parser.ParseException;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

/**
 * I'm quite sure I should parse something here. But what?
 *
 * @author Ocke Janssen
 */
public class FormatConditionReadHandler extends AbstractXmlReadHandler
{

    private final ReportElement element;

    public FormatConditionReadHandler(final ReportElement element)
    {
        if (element == null)
        {
            throw new NullPointerException();
        }
        this.element = element;
    }

    protected void startParsing(final Attributes attrs) throws SAXException
    {
        super.startParsing(attrs);


        final String formula =
                attrs.getValue(OfficeNamespaces.OOREPORT_NS, "formula");
        if (formula == null)
        {
            throw new ParseException("Required attribute 'formula' is missing.", getLocator());
        }
        final String stylename =
                attrs.getValue(OfficeNamespaces.OOREPORT_NS, OfficeToken.STYLE_NAME);
        if (stylename == null)
        {
            throw new ParseException("Required attribute 'style-name' is missing.", getLocator());
        }
        final FormulaExpression valueExpression = new FormulaExpression();
        valueExpression.setFormula(formula);

        final String enabledText = attrs.getValue(OfficeNamespaces.OOREPORT_NS, "enabled");
        final boolean enabled = (enabledText == null || OfficeToken.TRUE.equals(enabledText));
        final FormatCondition formatCondition =
                new FormatCondition(valueExpression, stylename, enabled);
        element.addFormatCondition(formatCondition);

    }

    /**
     * Returns the object for this element or null, if this element does not
     * create an object.
     *
     * @return the object.
     */
    public Object getObject()
            throws SAXException
    {
        return element;
    }
}
