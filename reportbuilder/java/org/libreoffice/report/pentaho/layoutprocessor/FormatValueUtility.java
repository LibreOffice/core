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

package org.libreoffice.report.pentaho.layoutprocessor;

import org.libreoffice.report.OfficeToken;
import org.libreoffice.report.pentaho.OfficeNamespaces;
import org.libreoffice.report.pentaho.model.FormattedTextElement;
import org.libreoffice.report.pentaho.model.OfficeGroupSection;
import org.libreoffice.report.pentaho.model.ReportElement;
import java.math.BigDecimal;

import java.sql.Time;

import java.text.SimpleDateFormat;

import java.util.Date;

import org.jfree.layouting.util.AttributeMap;
import org.jfree.report.DataFlags;
import org.jfree.report.DataRow;
import org.jfree.report.DataSourceException;
import org.jfree.report.data.DefaultDataFlags;
import org.jfree.report.expressions.Expression;
import org.jfree.report.expressions.FormulaExpression;
import org.jfree.report.flow.FlowController;
import org.jfree.report.flow.layoutprocessor.LayoutController;
import org.jfree.report.flow.layoutprocessor.LayoutControllerUtil;
import org.jfree.report.flow.layoutprocessor.SectionLayoutController;
import org.jfree.report.structure.Element;
import org.jfree.report.structure.Group;

import org.pentaho.reporting.libraries.formula.lvalues.ContextLookup;
import org.pentaho.reporting.libraries.formula.lvalues.LValue;
import org.pentaho.reporting.libraries.formula.util.HSSFDateUtil;

/**
 * Creation-Date: 06.06.2007, 17:03:30
 *
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
            ret = HSSFDateUtil.getExcelDate((Date) value).toString();
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
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, VALUE_TYPE, "void");
        }
        return ret;
    }

    public static void applyValueForCell(final Object value, final AttributeMap variableSection, final String valueType)
    {
        if (value instanceof Time)
        {
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, VALUE_TYPE, "time");
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, "time-value", formatTime((Time) value));
        }
        else if (value instanceof java.sql.Date)
        {
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, VALUE_TYPE, "date");
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, "date-value", formatDate((Date) value));
        }
        else if (value instanceof Date)
        {
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, VALUE_TYPE, "date");
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, "date-value", formatDate((Date) value));
        }
        else if (value instanceof BigDecimal)
        {
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, VALUE_TYPE, "float");
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, VALUE, String.valueOf(value));
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
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, VALUE_TYPE, "void");
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

    public static boolean shouldPrint(final LayoutController ref, final ReportElement text)
        throws DataSourceException
    {
        final boolean isValueChanged;
        if (ref instanceof AbstractReportElementLayoutController)
            isValueChanged=((AbstractReportElementLayoutController)ref).isValueChanged();
        else if (ref instanceof TableCellLayoutController)
            isValueChanged=((TableCellLayoutController)ref).isValueChanged();
        else
            throw new AssertionError("org.libreoffice.report.pentaho.layoutprocessor.FormatValueUtility.shouldPrint expects an implementor of isValueChanged as first argument");

        // Tests we have to perform:
        // 1. If repeated values are supposed to be printed, then print.
        //    (this is always the case for static text and static elements)
        // 2. If value changed, then print.
        // 3. If (printing should be forced on group change AND group changed), then print
        if ( !(    isValueChanged
                || text.isPrintRepeatedValues()
                || ( text.isPrintWhenGroupChange() && isGroupChanged(ref) )))
        {
            return false;
        }

        final Expression dc = text.getDisplayCondition();
        if (dc != null)
        {
            final Object o = LayoutControllerUtil.evaluateExpression(ref.getFlowController(), text, dc);
            if (Boolean.FALSE.equals(o))
            {
                return false;
            }
        }

        return true;
    }

    public static boolean isGroupChanged(LayoutController ref)
    {
        // search the group.
        final SectionLayoutController slc = findGroup(ref);
        if (slc == null)
        {
            // Always print the content of the report header and footer and
            // the page header and footer.
            return true;
        }

        // we are in the first iteration, so yes, the group has changed recently.
        return slc.getIterationCount() == 0;
    }

    public static SectionLayoutController findGroup(LayoutController ref)
    {
        LayoutController parent = ref.getParent();
        boolean skipNext = false;
        while (parent != null)
        {
            if (!(parent instanceof SectionLayoutController))
            {
                parent = parent.getParent();
            }
            else
            {
                final SectionLayoutController slc = (SectionLayoutController) parent;
                final Element element = slc.getElement();
                if (element instanceof OfficeGroupSection)
                {
                    // This is a header or footer. So we take the next group instead.
                    skipNext = true;
                    parent = parent.getParent();
                }
                else if (!(element instanceof Group))
                {
                    parent = parent.getParent();
                }
                else if (skipNext)
                {
                    skipNext = false;
                    parent = parent.getParent();
                }
                else
                {
                    return (SectionLayoutController) parent;
                }
            }
        }
        return null;
    }

    public static boolean isReferenceChanged(LayoutController ref, final LValue lValue)
    {
        if (lValue instanceof ContextLookup)
        {
            final ContextLookup rval = (ContextLookup) lValue;
            final String s = rval.getName();
            final DataRow view = ref.getFlowController().getMasterRow().getGlobalView();
            try
            {
                final DataFlags flags = view.getFlags(s);
                if (flags != null && flags.isChanged())
                {
                    return true;
                }
            }
            catch (DataSourceException e)
            {
                // ignore .. assume that the reference has not changed.
            }
        }
        final LValue[] childValues = lValue.getChildValues();
        for (int i = 0; i < childValues.length; i++)
        {
            final LValue value = childValues[i];
            if (isReferenceChanged(ref, value))
            {
                return true;
            }
        }
        return false;
    }

}
