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
import org.libreoffice.report.pentaho.model.FormatCondition;
import org.libreoffice.report.pentaho.model.FormattedTextElement;
import org.libreoffice.report.pentaho.model.ReportElement;

import org.jfree.layouting.util.AttributeMap;
import org.jfree.report.DataFlags;
import org.jfree.report.DataSourceException;
import org.jfree.report.expressions.Expression;
import org.jfree.report.expressions.FormulaExpression;
import org.jfree.report.flow.FlowController;
import org.jfree.report.flow.ReportTarget;
import org.jfree.report.flow.layoutprocessor.LayoutControllerUtil;
import org.jfree.report.flow.layoutprocessor.SectionLayoutController;
import org.jfree.report.structure.Element;
import org.jfree.report.structure.Node;
import org.jfree.report.structure.Section;
import org.pentaho.reporting.libraries.formula.Formula;
import org.pentaho.reporting.libraries.formula.lvalues.LValue;
import org.pentaho.reporting.libraries.formula.parser.ParseException;

import org.pentaho.reporting.libraries.base.util.ObjectUtilities;

/**
 * Before writing the table cell, we have to evaluate the children of the cell. The cell itself can either be empty or it
 * has a one ore more paragraphs inside. The paragraph contains a single report element, but may contain additional
 * other content.
 *
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
            // #i114108#: except on form elements, the only value-type that can
            // occur without an accompanying value attribute is "string"
            else if (!"string".equals(valueType))
            {
                attributeMap.setAttribute(OfficeNamespaces.OFFICE_NS,
                    FormatValueUtility.VALUE_TYPE, "void");
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
        if (!FormatValueUtility.shouldPrint(this, element))
        {
            attributeMap.setAttribute(OfficeNamespaces.OFFICE_NS,
                                      FormatValueUtility.VALUE_TYPE, "void");
            return null;
        }
        return FormatValueUtility.computeDataFlag(element, getFlowController());
    }

    public boolean isValueChanged()
    {
        try
        {
            final Section cell = (Section) getElement();
            final FormattedTextElement element = findFormattedTextElement(cell);
            if (element == null)
                return false;
            else
            {
                final FormulaExpression formulaExpression = element.getValueExpression();
                final Formula formula = formulaExpression.getCompiledFormula();
                final LValue lValue = formula.getRootReference();
                return FormatValueUtility.isReferenceChanged(this, lValue);
            }
        }
        catch (final ParseException e)
        {
            //LOGGER.debug("Parse Exception", e);
            return false;
        }
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
