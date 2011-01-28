/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
package com.sun.star.report.pentaho.layoutprocessor;

import com.sun.star.report.OfficeToken;
import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.pentaho.model.FormattedTextElement;
import java.math.BigDecimal;

import java.sql.Time;

import java.text.SimpleDateFormat;

import java.util.Date;

import org.jfree.layouting.util.AttributeMap;
import org.jfree.report.DataFlags;
import org.jfree.report.DataSourceException;
import org.jfree.report.data.DefaultDataFlags;
import org.jfree.report.expressions.FormulaExpression;
import org.jfree.report.flow.FlowController;
import org.jfree.report.flow.layoutprocessor.LayoutControllerUtil;

import org.pentaho.reporting.libraries.formula.util.HSSFDateUtil;

/**
 * Creation-Date: 06.06.2007, 17:03:30
 *
 * @author Thomas Morgner
 */
public class FormatValueUtility
{

    private static final String BOOLEAN_VALUE = "boolean-value";
    private static final String STRING_VALUE = "string-value";
    public static final String VALUE_TYPE = "value-type";
    public static final String VALUE = "value";
    private static SimpleDateFormat dateFormat;
    private static SimpleDateFormat timeFormat;

    private FormatValueUtility()
    {
    }

    public static String applyValueForVariable(final Object value, final AttributeMap variableSection)
    {
        String ret = null;
        if (value instanceof Time)
        {
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, VALUE_TYPE, "time");
            ret = formatTime((Time) value);
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, "time-value", ret);
        }
        else if (value instanceof java.sql.Date)
        {
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, VALUE_TYPE, "date");
            ret = formatDate((Date) value);
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, "date-value", ret);
        }
        else if (value instanceof Date)
        {
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, VALUE_TYPE, "float");
            ret = HSSFDateUtil.getExcelDate((Date) value, false, 2).toString();
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, VALUE, ret);
        }
        else if (value instanceof Number)
        {
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, VALUE_TYPE, "float");
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, VALUE, String.valueOf(value));
        }
        else if (value instanceof Boolean)
        {
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, VALUE_TYPE, "boolean");
            if (Boolean.TRUE.equals(value))
            {
                variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, BOOLEAN_VALUE, OfficeToken.TRUE);
            }
            else
            {
                variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, BOOLEAN_VALUE, OfficeToken.FALSE);
            }
        }
        else if (value != null)
        {
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, VALUE_TYPE, "string");
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, STRING_VALUE, String.valueOf(value));
        }
        else
        {
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, VALUE_TYPE, "string");
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, STRING_VALUE, "");
        }
        return ret;
    }

    public static void applyValueForCell(final Object value, final AttributeMap variableSection, final String valueType)
    {
        if (value instanceof Time)
        {
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, "time-value", formatTime((Time) value));
        }
        else if (value instanceof java.sql.Date)
        {
            if ("float".equals(valueType))//@see http://qa.openoffice.org/issues/show_bug.cgi?id=108954
            {
                variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, VALUE, HSSFDateUtil.getExcelDate((Date) value, false, 2).toString());
            }
            else
            {
                variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, "date-value", formatDate((Date) value));
            }
        }
        else if (value instanceof Date)
        {
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, VALUE_TYPE, "float");
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, VALUE, HSSFDateUtil.getExcelDate((Date) value, false, 2).toString());
        }
        else if (value instanceof BigDecimal)
        {
            if ("date".equals(valueType))
            {
                variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, "date-value", formatDate(HSSFDateUtil.getJavaDate((BigDecimal) value, false, 0)));
            }
            else
            {
                variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, VALUE, String.valueOf(value));
            }
        }
        else if (value instanceof Number)
        {
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, VALUE, String.valueOf(value));
        }
        else if (value instanceof Boolean)
        {
            if ("float".equals(valueType))
            {
                float fvalue = Boolean.TRUE.equals(value) ? 1 : 0;
                variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, VALUE, String.valueOf(fvalue));
            }
            else
            {
                if (Boolean.TRUE.equals(value))
                {
                    variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, BOOLEAN_VALUE, OfficeToken.TRUE);
                }
                else
                {
                    variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, BOOLEAN_VALUE, OfficeToken.FALSE);
                }
            }
        }
        else if (value != null)
        {
            try
            {
                final Float number = Float.valueOf(String.valueOf(value));
                applyValueForCell(number, variableSection, valueType);
                return;
            }
            catch (NumberFormatException e)
            {
            }
            if (!"string".equals(valueType))
            {
                variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, VALUE_TYPE, "string");
                //variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, VALUE, String.valueOf(value));
            }
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, STRING_VALUE, String.valueOf(value));
        }
        else
        {
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, STRING_VALUE, "");
        }
    }

    private static synchronized String formatDate(final Date date)
    {
        if (dateFormat == null)
        {
            dateFormat = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss'.'S'Z'");
        }
        return dateFormat.format(date);
    }

    private static synchronized String formatTime(final Date date)
    {
        if (timeFormat == null)
        {
            timeFormat = new SimpleDateFormat("'PT'HH'H'mm'M'ss'S'");
        }
        return timeFormat.format(date);
    }

    public static DataFlags computeDataFlag(final FormattedTextElement element,
            final FlowController flowController)
            throws DataSourceException
    {
        // here it is relatively easy. We have to evaluate the expression, convert
        // the result into a string, and print that string.
        final FormulaExpression formulaExpression = element.getValueExpression();
        final Object result = LayoutControllerUtil.evaluateExpression(flowController, element, formulaExpression);
        if (result == null)
        {
            // ignore it. Ignoring it is much better than printing 'null'.
            // LOGGER.debug("Formula '" + formulaExpression.getFormula() + "' evaluated to null.");
            return null;
        }
        else if (result instanceof DataFlags)
        {
            return (DataFlags) result;
        }
        else
        {
            return new DefaultDataFlags(null, result, true);
        }
    }
}
