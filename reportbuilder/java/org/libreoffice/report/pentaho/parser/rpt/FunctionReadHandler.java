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

import org.jfree.report.expressions.Expression;
import org.jfree.report.expressions.FormulaExpression;
import org.jfree.report.expressions.FormulaFunction;

import org.pentaho.reporting.libraries.xmlns.parser.AbstractXmlReadHandler;
import org.pentaho.reporting.libraries.xmlns.parser.ParseException;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

/**
 * Parses a named expression. These expressions are encountered on reports and
 * groups and compute global values. Expressions must have an unique name.
 *
 */
public class FunctionReadHandler extends AbstractXmlReadHandler
{

    private Expression expression;

    public FunctionReadHandler()
    {
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
        final String formula = attrs.getValue(OfficeNamespaces.OOREPORT_NS, "formula");
        if (formula == null)
        {
            throw new ParseException("Required attribute 'formula' is missing", getLocator());
        }

        final String name = attrs.getValue(OfficeNamespaces.OOREPORT_NS, "name");
        if (name == null)
        {
            throw new ParseException("Required attribute 'name' is missing", getLocator());
        }
        final String initialFormula = attrs.getValue(OfficeNamespaces.OOREPORT_NS, "initial-formula");
        final String deepTraversing = attrs.getValue(OfficeNamespaces.OOREPORT_NS, "deep-traversing");

        if (initialFormula != null)
        {
            final FormulaFunction function = new FormulaFunction();
            function.setInitial(initialFormula);
            function.setFormula(formula);
            this.expression = function;
        }
        else
        {
            final FormulaExpression expression = new FormulaExpression();
            expression.setFormula(formula);
            this.expression = expression;
        }

        expression.setName(name);
        expression.setDeepTraversing(OfficeToken.TRUE.equals(deepTraversing));
        final String preEvaluated = attrs.getValue(OfficeNamespaces.OOREPORT_NS, "pre-evaluated");
        expression.setPrecompute(OfficeToken.TRUE.equals(preEvaluated));
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
        return getExpression();
    }

    public Expression getExpression()
    {
        return expression;
    }
}
