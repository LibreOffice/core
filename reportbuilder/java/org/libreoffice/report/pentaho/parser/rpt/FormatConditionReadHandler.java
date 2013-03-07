/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package org.libreoffice.report.pentaho.parser.rpt;

import org.libreoffice.report.OfficeToken;
import org.libreoffice.report.pentaho.OfficeNamespaces;
import org.libreoffice.report.pentaho.model.FormatCondition;
import org.libreoffice.report.pentaho.model.ReportElement;

import org.jfree.report.expressions.FormulaExpression;

import org.pentaho.reporting.libraries.xmlns.parser.AbstractXmlReadHandler;
import org.pentaho.reporting.libraries.xmlns.parser.ParseException;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

/**
 * I'm quite sure I should parse something here. But what?
 *
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
