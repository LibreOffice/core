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

import com.sun.star.report.pentaho.OfficeNamespaces;

import org.jfree.report.expressions.FormulaFunction;
import org.jfree.report.structure.Element;

import org.pentaho.reporting.libraries.xmlns.parser.AbstractXmlReadHandler;

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

    private final Element element;

    public ConditionalPrintExpressionReadHandler(final Element element)
    {
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
