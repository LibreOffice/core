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
