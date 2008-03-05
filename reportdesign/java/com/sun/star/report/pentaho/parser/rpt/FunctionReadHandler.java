/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FunctionReadHandler.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:42:54 $
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

import org.jfree.xmlns.parser.AbstractXmlReadHandler;
import org.jfree.xmlns.parser.ParseException;
import org.jfree.report.expressions.Expression;
import org.jfree.report.expressions.FormulaFunction;
import org.jfree.report.expressions.FormulaExpression;
import org.xml.sax.SAXException;
import org.xml.sax.Attributes;
import com.sun.star.report.pentaho.OfficeNamespaces;

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
        final String initialFormula = attrs.getValue(OfficeNamespaces.OOREPORT_NS, "initial-formula");
        final String name = attrs.getValue(OfficeNamespaces.OOREPORT_NS, "name");
        if (name == null)
        {
            throw new ParseException("Required attribute 'name' is missing", getLocator());
        }
        final String preEvaluated = attrs.getValue(OfficeNamespaces.OOREPORT_NS, "pre-evaluated");
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
        expression.setDeepTraversing("true".equals(deepTraversing));
        expression.setPrecompute("true".equals(preEvaluated));
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
