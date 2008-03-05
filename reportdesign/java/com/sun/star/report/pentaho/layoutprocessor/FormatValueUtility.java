package com.sun.star.report.pentaho.layoutprocessor;

import java.text.SimpleDateFormat;
import java.util.Date;

import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.pentaho.model.FormattedTextElement;
import org.jfree.layouting.util.AttributeMap;
import org.jfree.report.DataFlags;
import org.jfree.report.DataSourceException;
import org.jfree.report.data.DefaultDataFlags;
import org.jfree.report.expressions.FormulaExpression;
import org.jfree.report.flow.FlowController;
import org.jfree.report.flow.layoutprocessor.LayoutControllerUtil;
import org.jfree.util.Log;

/**
 * Creation-Date: 06.06.2007, 17:03:30
 *
 * @author Thomas Morgner
 */
public class FormatValueUtility
{
    private static final String BOOLEAN_VALUE = "boolean-value";
    private static final String STRING_VALUE = "string-value";

    private static final String VALUE_TYPE = "value-type";
    private static SimpleDateFormat dateFormat;

    private FormatValueUtility()
    {
    }

    public static void applyValueForVariable(final Object value, final AttributeMap variableSection)
    {
        if (value instanceof Date)
        {
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, VALUE_TYPE, "date");
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, "date-value", formatDate((Date) value));
        }
        else if (value instanceof Number)
        {
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, VALUE_TYPE, "float");
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, "value", String.valueOf(value));
        }
        else if (value instanceof Boolean)
        {
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, VALUE_TYPE, "boolean");
            if (Boolean.TRUE.equals(value))
            {
                variableSection.setAttribute(OfficeNamespaces.OFFICE_NS,BOOLEAN_VALUE, "true");
            }
            else
            {
                variableSection.setAttribute(OfficeNamespaces.OFFICE_NS,BOOLEAN_VALUE, "false");
            }
        }
        else if (value != null)
        {
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, VALUE_TYPE, "string");
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS,STRING_VALUE, String.valueOf(value));
        }
        else
        {
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, VALUE_TYPE, "string");
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS,STRING_VALUE, "");
        }
    }

    public static void applyValueForCell(final Object value, final AttributeMap variableSection)
    {
        if (value instanceof Date)
        {
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, "date-value", formatDate((Date) value));
        }
        else if (value instanceof Number)
        {
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS, "value", String.valueOf(value));
        }
        else if (value instanceof Boolean)
        {
            if (Boolean.TRUE.equals(value))
            {
                variableSection.setAttribute(OfficeNamespaces.OFFICE_NS,BOOLEAN_VALUE, "true");
            }
            else
            {
                variableSection.setAttribute(OfficeNamespaces.OFFICE_NS,BOOLEAN_VALUE, "false");
            }
        }
        else if (value != null)
        {
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS,STRING_VALUE, String.valueOf(value));
        }
        else
        {
            variableSection.setAttribute(OfficeNamespaces.OFFICE_NS,STRING_VALUE, "");
        }
    }

    private static synchronized String formatDate(final Date date)
    {
        if (dateFormat == null)
        {
            dateFormat = new SimpleDateFormat("yyyy-MM-dd'T'hh:mm:ss'.'S'Z'");
        }
        return dateFormat.format(date);
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
            Log.debug("Formula '" + formulaExpression.getFormula() + "' evaluated to null.");
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
