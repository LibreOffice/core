/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ConditionalPrintExpressionReadHandler.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:41:20 $
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

import com.sun.star.report.pentaho.OfficeNamespaces;
import org.jfree.report.expressions.FormulaFunction;
import org.jfree.report.structure.Element;
import org.jfree.xmlns.parser.AbstractXmlReadHandler;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

/**
 * Handles the 'report:conditional-print-expression' element that can appear
 * in all report elements and all root-level sections.
 *
 * @author Thomas Morgner
 * @since 02.03.2007
 */
public class ConditionalPrintExpressionReadHandler
        extends AbstractXmlReadHandler
{

    private Element element;

    public ConditionalPrintExpressionReadHandler(final Element element)
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
        final String formula = attrs.getValue(OfficeNamespaces.OOREPORT_NS, "formula");
        if (formula != null)
        {
            final FormulaFunction valueExpression = new FormulaFunction();
            valueExpression.setFormula(formula);
            element.setDisplayCondition(valueExpression);
        }

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
