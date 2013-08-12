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
import org.libreoffice.report.pentaho.model.OfficeDocument;
import org.libreoffice.report.pentaho.model.OfficeStyle;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import org.jfree.layouting.util.AttributeMap;
import org.jfree.report.DataFlags;
import org.jfree.report.DataSourceException;
import org.jfree.report.JFreeReportInfo;
import org.jfree.report.ReportDataFactoryException;
import org.jfree.report.ReportProcessingException;
import org.jfree.report.expressions.FormulaExpression;
import org.jfree.report.flow.ReportTarget;
import org.jfree.report.flow.layoutprocessor.LayoutController;
import org.jfree.report.structure.Element;

import org.pentaho.reporting.libraries.formula.Formula;
import org.pentaho.reporting.libraries.formula.lvalues.LValue;
import org.pentaho.reporting.libraries.formula.parser.ParseException;

/**
 * Todo: Document me!
 *
 * @since 05.03.2007
 */
public class FormattedTextLayoutController
        extends AbstractReportElementLayoutController
{

    private static final Log LOGGER = LogFactory.getLog(FormattedTextLayoutController.class);

    public FormattedTextLayoutController()
    {
    }

    private VariablesCollection getVariablesCollection()
    {
        LayoutController parent = getParent();
        while (parent != null)
        {
            if (parent instanceof OfficeRepeatingStructureLayoutController)
            {
                final OfficeRepeatingStructureLayoutController orslc =
                        (OfficeRepeatingStructureLayoutController) parent;
                if (orslc.isNormalFlowProcessing())
                {
                    return null;
                }

                return orslc.getVariablesCollection();
            }
            parent = parent.getParent();
        }
        return null;
    }

    public boolean isValueChanged()
    {
        try
        {
            final FormattedTextElement element = (FormattedTextElement) getNode();
            final FormulaExpression formulaExpression = element.getValueExpression();
            final Formula formula = formulaExpression.getCompiledFormula();
            final LValue lValue = formula.getRootReference();
            return FormatValueUtility.isReferenceChanged(this, lValue);
        }
        catch (final ParseException e)
        {
            LOGGER.debug("Parse Exception", e);
            return false;
        }
    }

    protected LayoutController delegateContentGeneration(final ReportTarget target)
            throws ReportProcessingException, ReportDataFactoryException,
            DataSourceException
    {
        final FormattedTextElement element = (FormattedTextElement) getNode();
        // LEM 20130812 I have absolutely no clue why it wants to go via
        // a variable like that. It complicates things, is fragile
        // (because the variable-set is done in *every* detail section
        //  again and again. This in itself is not that bad, but when
        //  the detail section is of height zero, the "set" us never done...
        //  and this whole schema fails). For now, keep the code in case
        //  something break. If we survive the 4.2 cycle (in its entirety)
        //  without regression traced to this, then remove it (for 4.4 or
        //  something like that).
        // final VariablesCollection vc = getVariablesCollection();
        // if (vc != null)
        // {
        //     final String name = vc.addVariable(element);
        //     final AttributeMap variablesGet = new AttributeMap();
        //     variablesGet.setAttribute(JFreeReportInfo.REPORT_NAMESPACE,
        //             Element.TYPE_ATTRIBUTE, "variable-get");
        //     variablesGet.setAttribute(JFreeReportInfo.REPORT_NAMESPACE,
        //             Element.NAMESPACE_ATTRIBUTE, OfficeNamespaces.TEXT_NS);
        //     variablesGet.setAttribute(OfficeNamespaces.TEXT_NS, "name", name);

        //     final String dataStyleName = computeValueStyle();
        //     if (dataStyleName != null)
        //     {
        //         variablesGet.setAttribute(OfficeNamespaces.STYLE_NS, "data-style-name", dataStyleName);
        //     }

        //     final String valueType = computeValueType();
        //     variablesGet.setAttribute(OfficeNamespaces.OFFICE_NS, FormatValueUtility.VALUE_TYPE, valueType);
        //     target.startElement(variablesGet);

        //     target.endElement(variablesGet);
        // }
        // else
        {
            final DataFlags df = FormatValueUtility.computeDataFlag(element, getFlowController());
            if (df != null)
            {
                if (df.getValue() instanceof String)
                {
                    target.processContent(df);
                }
                else //@see http://qa.openoffice.org/issues/show_bug.cgi?id=108954
                {
                    Element cell = getParentTableCell();
                    if (cell != null && "string".equals(cell.getAttribute(OfficeNamespaces.OFFICE_NS, FormatValueUtility.VALUE_TYPE)))
                    {
                        target.processContent(df);
                    }
                }
            }
        }

        return join(getFlowController());
    }

    private OfficeDocument getDocument()
    {
        LayoutController parent = getParent();
        while (parent != null)
        {
            final Object node = parent.getNode();
            if (node instanceof OfficeDocument)
            {
                return (OfficeDocument) node;
            }
            parent = parent.getParent();
        }
        return null;
    }

    private Element getParentTableCell()
    {
        LayoutController parent = getParent();
        while (parent != null)
        {
            if (parent instanceof TableCellLayoutController)
            {
                final TableCellLayoutController cellController = (TableCellLayoutController) parent;
                return cellController.getElement();
            }
            parent = parent.getParent();
        }
        return null;
    }

    private String computeValueStyle()
    {
        final Element tce = getParentTableCell();
        if (tce == null)
        {
            return null;
        }

        final String cellStyleName = (String) tce.getAttribute(OfficeNamespaces.TABLE_NS, OfficeToken.STYLE_NAME);
        if (cellStyleName == null)
        {
            return null;
        }
        final OfficeDocument document = getDocument();
        if (document == null)
        {
            return null;
        }

        final OfficeStyle style = document.getStylesCollection().getStyle("table-cell", cellStyleName);
        return (String) style.getAttribute(OfficeNamespaces.STYLE_NS, "data-style-name");
    }

    private String computeValueType()
    {
        final Element tce = getParentTableCell();
        if (tce == null)
        {
            // NO particular format means: Fallback to string and hope and pray ..
            throw new IllegalStateException("A formatted text element must be a child of a Table-Cell.");
        }

        final String type = (String) tce.getAttribute(OfficeNamespaces.OFFICE_NS, FormatValueUtility.VALUE_TYPE);
        if (type == null)
        {
            LOGGER.error("The Table-Cell does not have a office:value attribute defined. Your content will be messed up.");
            return "string";
        }
        return type;
    }
}
