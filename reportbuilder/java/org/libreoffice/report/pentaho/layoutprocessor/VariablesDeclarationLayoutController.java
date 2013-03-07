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

import org.libreoffice.report.pentaho.OfficeNamespaces;
import org.libreoffice.report.pentaho.model.FormattedTextElement;

import java.text.SimpleDateFormat;

import java.util.Date;

import org.jfree.layouting.util.AttributeMap;
import org.jfree.report.DataSourceException;
import org.jfree.report.JFreeReportInfo;
import org.jfree.report.ReportDataFactoryException;
import org.jfree.report.ReportProcessingException;
import org.jfree.report.expressions.FormulaExpression;
import org.jfree.report.flow.FlowController;
import org.jfree.report.flow.ReportTarget;
import org.jfree.report.flow.layoutprocessor.AbstractLayoutController;
import org.jfree.report.flow.layoutprocessor.LayoutController;
import org.jfree.report.flow.layoutprocessor.LayoutControllerUtil;
import org.jfree.report.structure.Element;

/**
 * Writes a full variables-declaration section.
 *
 * @since 20.03.2007
 */
public class VariablesDeclarationLayoutController
        extends AbstractLayoutController
{

    private boolean processed;

    public VariablesDeclarationLayoutController()
    {
    }

    private OfficeRepeatingStructureLayoutController getRepeatingParent()
    {
        LayoutController parent = getParent();
        while (parent != null)
        {
            if (parent instanceof OfficeRepeatingStructureLayoutController)
            {
                return (OfficeRepeatingStructureLayoutController) parent;
            }
            parent = parent.getParent();
        }
        return null;
    }

    /**
     * Advances the processing position.
     *
     * @param target the report target that receives generated events.
     * @return the new layout controller instance representing the new state.
     *
     * @throws org.jfree.report.DataSourceException
     *          if there was a problem reading data from the datasource.
     * @throws org.jfree.report.ReportProcessingException
     *          if there was a general problem during the report processing.
     * @throws org.jfree.report.ReportDataFactoryException
     *          if a query failed.
     */
    public LayoutController advance(final ReportTarget target)
            throws DataSourceException, ReportDataFactoryException,
            ReportProcessingException
    {
        if (processed)
        {
            throw new IllegalStateException("Already processed.");
        }

        final VariablesDeclarationLayoutController vlc =
                (VariablesDeclarationLayoutController) clone();
        vlc.processed = true;

        final OfficeRepeatingStructureLayoutController orslc = getRepeatingParent();
        if (orslc == null)
        {
            // There is no repeating parent. What the heck are we doing here ..
            return vlc;
        }

        final VariablesCollection collection = orslc.getVariablesCollection();
        if (collection.getVariablesCount() == 0)
        {
            // no processing necessary, as the header or footer contain no variables at all ..
            return vlc;
        }


        final Element node = (Element) getNode();
        final AttributeMap vdSection = node.getAttributeMap();
        target.startElement(vdSection);

        final FormattedTextElement[] variables = collection.getVariables();
        for (int i = 0; i < variables.length; i++)
        {
            final FormattedTextElement variable = variables[i];
            final String varName = collection.getNamePrefix() + (i + 1);
            final AttributeMap map = generateVariableSetSection(variable);
            map.setAttribute(OfficeNamespaces.TEXT_NS, "name", varName);
            target.startElement(map);
            target.endElement(map);

        }
        target.endElement(vdSection);
        return vlc;
    }

    private AttributeMap generateVariableSetSection(final FormattedTextElement variable)
            throws DataSourceException
    {
        final AttributeMap variableSection = new AttributeMap();
        variableSection.setAttribute(JFreeReportInfo.REPORT_NAMESPACE, Element.NAMESPACE_ATTRIBUTE, OfficeNamespaces.TEXT_NS);
        variableSection.setAttribute(JFreeReportInfo.REPORT_NAMESPACE, Element.TYPE_ATTRIBUTE, "variable-set");
        variableSection.setAttribute(OfficeNamespaces.TEXT_NS, "display", "none");

        final FormulaExpression valueExpression = variable.getValueExpression();
        final Object value = LayoutControllerUtil.evaluateExpression(getFlowController(), variable, valueExpression);
        String formula = FormatValueUtility.applyValueForVariable(value, variableSection);
        if (formula == null)
        {
            formula = "" + value;
        }
        if (value instanceof java.sql.Date)
        {
            final Date date = (Date) value;
            final SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy;MM;dd");
            formula = "Date(" + dateFormat.format(date) + ")";
        }
        variableSection.setAttribute(OfficeNamespaces.TEXT_NS, "formula", "ooow:" + formula);

        return variableSection;
    }

    /**
     * Checks, whether the layout controller would be advanceable. If this method
     * returns true, it is generally safe to call the 'advance()' method.
     *
     * @return true, if the layout controller is advanceable, false otherwise.
     */
    public boolean isAdvanceable()
    {
        return !processed;
    }

    /**
     * Joins with a delegated process flow. This is generally called from a child
     * flow and should *not* (I mean it!) be called from outside. If you do,
     * you'll suffer.
     *
     * @param flowController the flow controller of the parent.
     * @return the joined layout controller that incorperates all changes from the
     *         delegate.
     */
    public LayoutController join(final FlowController flowController)
            throws DataSourceException, ReportDataFactoryException,
            ReportProcessingException
    {
        throw new UnsupportedOperationException("Join is not supported in this layout controller");
    }
}
