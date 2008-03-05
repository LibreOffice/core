/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FormattedTextLayoutController.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:31:38 $
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


package com.sun.star.report.pentaho.layoutprocessor;

import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.pentaho.model.FormattedTextElement;
import com.sun.star.report.pentaho.model.OfficeDocument;
import com.sun.star.report.pentaho.model.OfficeStyle;
import org.jfree.formula.Formula;
import org.jfree.formula.lvalues.LValue;
import org.jfree.formula.parser.ParseException;
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
import org.jfree.util.Log;

/**
 * Todo: Document me!
 *
 * @author Thomas Morgner
 * @since 05.03.2007
 */
public class FormattedTextLayoutController
    extends AbstractReportElementLayoutController
{
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

  protected boolean isValueChanged()
  {
    try
    {
      final FormattedTextElement element = (FormattedTextElement) getNode();
      final FormulaExpression formulaExpression = element.getValueExpression();
      final Formula formula = formulaExpression.getCompiledFormula();
      final LValue lValue = formula.getRootReference();
      return isReferenceChanged(lValue);
    }
    catch (final ParseException e)
    {
      Log.debug("Parse Exception", e);
      return false;
    }
  }

  protected LayoutController delegateContentGeneration(final ReportTarget target)
      throws ReportProcessingException, ReportDataFactoryException,
      DataSourceException
  {
    final FormattedTextElement element = (FormattedTextElement) getNode();
    final VariablesCollection vc = getVariablesCollection();
    if (vc != null)
    {
      final String name = vc.addVariable(element);
      final AttributeMap variablesGet = new AttributeMap();
      variablesGet.setAttribute(JFreeReportInfo.REPORT_NAMESPACE,
          Element.TYPE_ATTRIBUTE, "variable-get");
      variablesGet.setAttribute(JFreeReportInfo.REPORT_NAMESPACE,
          Element.NAMESPACE_ATTRIBUTE, OfficeNamespaces.TEXT_NS);
      variablesGet.setAttribute(OfficeNamespaces.TEXT_NS, "name", name);
      //variablesGet.setAttribute(OfficeNamespaces.TEXT_NS, "display", "value");

      final String dataStyleName = computeValueStyle();
      if (dataStyleName != null)
      {
        variablesGet.setAttribute(OfficeNamespaces.STYLE_NS, "data-style-name", dataStyleName);
      }

      final String valueType = computeValueType();
      variablesGet.setAttribute(OfficeNamespaces.OFFICE_NS, "value-type", valueType);
      target.startElement(variablesGet);

      target.endElement(variablesGet);
    }
    else
    {

      final DataFlags df = FormatValueUtility.computeDataFlag(element, getFlowController());
      if (df != null)
      {
        target.processContent(df);
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

    final String cellStyleName = (String) tce.getAttribute(OfficeNamespaces.TABLE_NS, "style-name");
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

    final String type = (String) tce.getAttribute(OfficeNamespaces.OFFICE_NS, "value-type");
    if (type == null)
    {
      Log.error("The Table-Cell does not have a office:value attribute defined. Your content will be messed up.");
      return "string";
    }
    return type;
  }
}
