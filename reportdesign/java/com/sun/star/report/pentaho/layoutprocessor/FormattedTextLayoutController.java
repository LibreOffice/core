/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FormattedTextLayoutController.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:05 $
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
import org.jfree.report.flow.layoutprocessor.ElementLayoutController;
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
//  private Boolean inRepeatingSection;

  public FormattedTextLayoutController()
  {
  }

//  private boolean isInRepeatingSection ()
//  {
//    if (inRepeatingSection == null)
//    {
//      LayoutController parent = getParent();
//      while (parent != null && inRepeatingSection == null)
//      {
//        if (parent instanceof OfficeRepeatingStructureLayoutController)
//        {
//          final OfficeRepeatingStructureLayoutController orslc =
//              (OfficeRepeatingStructureLayoutController) parent;
//          if (orslc.isNormalFlowProcessing())
//          {
//            inRepeatingSection = Boolean.FALSE;
//          }
//          else
//          {
//            inRepeatingSection = Boolean.TRUE;
//          }
//        }
//        parent = parent.getParent();
//      }
//
//      if (inRepeatingSection == null)
//      {
//        inRepeatingSection = Boolean.FALSE;
//      }
//    }
//    return inRepeatingSection.booleanValue();
//  }

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
      Log.debug ("Parse Exception" , e);
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

  private Element getParentTableCell()
  {
    LayoutController parent = getParent();
    while (parent != null)
    {
      if (parent instanceof ElementLayoutController)
      {
        final ElementLayoutController cellController = (ElementLayoutController) parent;
        return cellController.getElement();
      }
      parent = parent.getParent();
    }
    return null;
  }

  private String computeValueType ()
  {
    final Element tce = getParentTableCell();
    if (tce == null)
    {
      // NO particular format means: Fallback to string and hope and pray ..
      return "string";
    }

    final String type = (String) tce.getAttribute(OfficeNamespaces.OFFICE_NS, "value-type");
    if (type == null)
    {
      return "string";
    }
    return type;
  }
}
