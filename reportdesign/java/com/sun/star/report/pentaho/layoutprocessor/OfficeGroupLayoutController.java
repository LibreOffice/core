/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OfficeGroupLayoutController.java,v $
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
import com.sun.star.report.pentaho.model.OfficeGroup;
import com.sun.star.report.pentaho.model.OfficeGroupSection;
import com.sun.star.report.pentaho.model.VariablesDeclarationSection;
import org.jfree.report.DataSourceException;
import org.jfree.report.ReportDataFactoryException;
import org.jfree.report.ReportProcessingException;
import org.jfree.report.flow.FlowController;
import org.jfree.report.flow.ReportTarget;
import org.jfree.report.flow.layoutprocessor.LayoutController;
import org.jfree.report.flow.layoutprocessor.SectionLayoutController;
import org.jfree.report.flow.layoutprocessor.ElementLayoutController;

/**
 * Todo: Document me!
 *
 * @author Thomas Morgner
 * @since 15.03.2007
 */
public class OfficeGroupLayoutController extends SectionLayoutController
    implements OfficeRepeatingStructureLayoutController
{
  public static final int STATE_PROCESS_REPEATING_HEADER = 0;
  public static final int STATE_PROCESS_REPEATING_FOOTER = 1;
  public static final int STATE_PROCESS_VARIABLES = 2;
  public static final int STATE_PROCESS_NORMAL_FLOW = 3;
  private boolean waitForJoin;
  private int state;
  private VariablesCollection variablesCollection;

  public OfficeGroupLayoutController()
  {
  }


  /**
   * Initializes the layout controller. This method is called exactly once. It
   * is the creators responsibility to call this method.
   * <p/>
   * Calling initialize after the first advance must result in a
   * IllegalStateException.
   *
   * @param node           the currently processed object or layout node.
   * @param flowController the current flow controller.
   * @param parent         the parent layout controller that was responsible for
   *                       instantiating this controller.
   * @throws org.jfree.report.DataSourceException
   *          if there was a problem reading data from the datasource.
   * @throws org.jfree.report.ReportProcessingException
   *          if there was a general problem during the report processing.
   * @throws org.jfree.report.ReportDataFactoryException
   *          if a query failed.
   */
  public void initialize(final Object node,
                         final FlowController flowController,
                         final LayoutController parent)
      throws DataSourceException, ReportDataFactoryException,
      ReportProcessingException
  {
    super.initialize(node, flowController, parent);
    state = OfficeGroupLayoutController.STATE_PROCESS_REPEATING_HEADER;
    variablesCollection = new VariablesCollection(computeVariablesPrefix());
  }


  protected LayoutController processContent(final ReportTarget target)
      throws DataSourceException, ReportProcessingException,
      ReportDataFactoryException
  {
    if (state == OfficeGroupLayoutController.STATE_PROCESS_REPEATING_HEADER)
    {
      final OfficeGroup group = (OfficeGroup) getElement();
      final OfficeGroupSection header =
          (OfficeGroupSection) group.findFirstChild
          (OfficeNamespaces.OOREPORT_NS, "group-header");

      final OfficeGroupLayoutController controller =
          (OfficeGroupLayoutController) clone();
      controller.state =
          OfficeGroupLayoutController.STATE_PROCESS_REPEATING_FOOTER;

      if (header == null || header.isRepeatSection() == false)
      {
        return controller;
      }

      controller.waitForJoin = true;
      return processChild(controller, header, getFlowController());
    }

    if (state == OfficeGroupLayoutController.STATE_PROCESS_REPEATING_FOOTER)
    {
      final OfficeGroup group = (OfficeGroup) getElement();
      final OfficeGroupSection footer =
          (OfficeGroupSection) group.findFirstChild
          (OfficeNamespaces.OOREPORT_NS, "group-footer");

      final OfficeGroupLayoutController controller =
          (OfficeGroupLayoutController) clone();
      controller.state =
          OfficeGroupLayoutController.STATE_PROCESS_VARIABLES;

      if (footer == null || footer.isRepeatSection() == false)
      {
        return controller;
      }

      controller.waitForJoin = true;
      return processChild(controller, footer, getFlowController());
    }

    if (state == OfficeGroupLayoutController.STATE_PROCESS_VARIABLES)
    {
      // todo: Fill the variables section with something sensible ..
      final VariablesDeclarationSection variables =
          new VariablesDeclarationSection();
      final OfficeGroupLayoutController controller =
          (OfficeGroupLayoutController) clone();
      controller.state =
          OfficeGroupLayoutController.STATE_PROCESS_NORMAL_FLOW;
      controller.waitForJoin = true;
      return processChild(controller, variables, getFlowController());
    }
    return super.processContent(target);
  }

  protected void resetSectionForRepeat()
  {
    super.resetSectionForRepeat();
    state = STATE_PROCESS_VARIABLES;
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
  {
    if (waitForJoin)
    {
      final OfficeGroupLayoutController derived = (OfficeGroupLayoutController) clone();
      derived.setProcessingState(ElementLayoutController.OPENED);
      derived.setFlowController(flowController);
      derived.waitForJoin = false;
      return derived;
    }
    return super.join(flowController);
  }

  public boolean isNormalFlowProcessing ()
  {
    return state == OfficeGroupLayoutController.STATE_PROCESS_NORMAL_FLOW;
  }

  private String computeVariablesPrefix()
  {
    int count = 0;
    LayoutController lc = this;
    while (lc != null)
    {
      if (lc instanceof OfficeGroupLayoutController)
      {
        count += 1;
      }
      lc = lc.getParent();
    }
    return "auto_group_" + count + "_";
  }

  public VariablesCollection getVariablesCollection()
  {
    return variablesCollection;
  }
}
