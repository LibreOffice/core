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
import com.sun.star.report.pentaho.model.FormatCondition;
import com.sun.star.report.pentaho.model.FormattedTextElement;
import com.sun.star.report.pentaho.model.ReportElement;

import org.jfree.layouting.util.AttributeMap;
import org.jfree.report.DataFlags;
import org.jfree.report.DataSourceException;
import org.jfree.report.expressions.Expression;
import org.jfree.report.flow.FlowController;
import org.jfree.report.flow.ReportTarget;
import org.jfree.report.flow.layoutprocessor.LayoutControllerUtil;
import org.jfree.report.flow.layoutprocessor.SectionLayoutController;
import org.jfree.report.structure.Element;
import org.jfree.report.structure.Node;
import org.jfree.report.structure.Section;

import org.pentaho.reporting.libraries.base.util.ObjectUtilities;

/**
 * Before writing the table cell, we have to evaluate the childs of the cell. The cell itself can either be empty or it
 * has a one ore more paragraphs inside. The paragraph contains a single report element, but may contain additional
 * other content.
 *
 * @author Thomas Morgner
 * @noinspection CloneableClassWithoutClone
 * @since 05.03.2007
 */
public class TableCellLayoutController extends SectionLayoutController
{

    public TableCellLayoutController()
    {
    }

    protected AttributeMap computeAttributes(final FlowController fc,
            final Element element,
            final ReportTarget target)
            throws DataSourceException
    {
        final AttributeMap attributeMap = new AttributeMap(super.computeAttributes(fc, element, target));
        final String definedStyle = (String) attributeMap.getAttribute(OfficeNamespaces.TABLE_NS, OfficeToken.STYLE_NAME);
        attributeMap.setAttribute(OfficeNamespaces.TABLE_NS, OfficeToken.STYLE_NAME, getDisplayStyleName((Section) element, definedStyle));

        try
        {
            final DataFlags value = computeValue(attributeMap);
            final String valueType = (String) attributeMap.getAttribute(OfficeNamespaces.OFFICE_NS, FormatValueUtility.VALUE_TYPE);
            if (value != null)
            {
                FormatValueUtility.applyValueForCell(value.getValue(), attributeMap, valueType);
            }
            else if ( "float".equals(valueType))
            {
                attributeMap.setAttribute(OfficeNamespaces.OFFICE_NS,
                    FormatValueUtility.VALUE, "NaN");
            }
            // #i114108#: except on form elements, the only value-type that can
            // occur without an accomanying value attribute is "string"
            else if (!"string".equals(valueType))
            {
                attributeMap.setAttribute(OfficeNamespaces.OFFICE_NS,
                    FormatValueUtility.VALUE_TYPE, "string");
            }
        }
        catch (Exception e)
        {
            // ignore ..
        }
        attributeMap.makeReadOnly();
        return attributeMap;
    }

    private DataFlags computeValue(final AttributeMap attributeMap) throws DataSourceException
    {
        // Search for the first FormattedTextElement
        final Section cell = (Section) getElement();
        final FormattedTextElement element = findFormattedTextElement(cell);
        if (element == null)
        {
            return null;
        }
        final Expression dc = element.getDisplayCondition();
        if (dc != null)
        {
            final Object o = LayoutControllerUtil.evaluateExpression(getFlowController(), element, dc);
            if (Boolean.FALSE.equals(o))
            {
                attributeMap.setAttribute(OfficeNamespaces.OFFICE_NS,
                    FormatValueUtility.VALUE_TYPE, "string");
                return null;
            }
        }
        return FormatValueUtility.computeDataFlag(element, getFlowController());
    }

    private FormattedTextElement findFormattedTextElement(final Section section)
    {
        final Node[] nodeArray = section.getNodeArray();
        for (int i = 0; i < nodeArray.length; i++)
        {
            final Node node = nodeArray[i];
            if (node instanceof FormattedTextElement)
            {
                return (FormattedTextElement) node;
            }
            else if (node instanceof Section)
            {
                final FormattedTextElement retval = findFormattedTextElement((Section) node);
                if (retval != null)
                {
                    return retval;
                }
            }
        }
        return null;
    }

    private String getDisplayStyleName(final Section section,
            final String defaultStyle)
    {
        if (!section.isEnabled() || section.getNodeCount() == 0)
        {
            return defaultStyle;
        }

        final Node[] nodes = section.getNodeArray();
        for (int i = 0; i < nodes.length; i++)
        {
            final Node child = nodes[i];
            if (child instanceof ReportElement && child.isEnabled())
            {
                final ReportElement element = (ReportElement) child;
                if (element.getFormatConditionCount() > 0)
                {
                    final Expression displayCond = element.getDisplayCondition();
                    if (displayCond != null)
                    {
                        try
                        {
                            if (Boolean.FALSE.equals(LayoutControllerUtil.evaluateExpression(getFlowController(), element, displayCond)))
                            {
                                continue;
                            }
                        }
                        catch (DataSourceException e)
                        {
                            // ignore silently ..
                        }
                    }

                    final FormatCondition[] conditions = element.getFormatConditions();
                    for (int j = 0; j < conditions.length; j++)
                    {
                        final FormatCondition formCond = conditions[j];
                        if (formCond.isEnabled())
                        {
                            try
                            {
                                final Object o = LayoutControllerUtil.evaluateExpression(getFlowController(), element, formCond.getFormula());
                                if (Boolean.TRUE.equals(o))
                                {
                                    return formCond.getStyleName();
                                }
                            }
                            catch (DataSourceException e)
                            {
                                // ignore silently ..
                            }
                        }
                    }
                }
            }

            if (child instanceof Section)
            {
                final String childFormatCondition =
                        getDisplayStyleName((Section) child, defaultStyle);
                if (!ObjectUtilities.equal(childFormatCondition, defaultStyle))
                {
                    return childFormatCondition;
                }
            }
        }
        return defaultStyle;
    }
}
