/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: FunctionReadHandler.java,v $
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

import org.pentaho.reporting.libraries.xmlns.parser.AbstractXmlReadHandler;
import org.pentaho.reporting.libraries.xmlns.parser.ParseException;
import org.jfree.report.expressions.Expression;
import org.jfree.report.expressions.FormulaFunction;
import org.jfree.report.expressions.FormulaExpression;
import org.xml.sax.SAXException;
import org.xml.sax.Attributes;
import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.OfficeToken;

/**
 * Parses a named expression. These expressions are encountered on reports and
 * groups and compute global values. Expressions must have an unique name.
 *
 * @author Thomas Morgner
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
