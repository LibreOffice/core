/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OfficeDetailLayoutController.java,v $
 * $Revision: 1.4 $
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

import com.sun.star.report.pentaho.model.VariablesDeclarationSection;
import org.jfree.report.DataSourceException;
import org.jfree.report.ReportDataFactoryException;
import org.jfree.report.ReportProcessingException;
import org.jfree.report.ReportData;
import org.jfree.report.data.GlobalMasterRow;
import org.jfree.report.data.ReportDataRow;
import org.jfree.report.flow.FlowController;
import org.jfree.report.flow.ReportTarget;
import org.jfree.report.flow.layoutprocessor.ElementLayoutController;
import org.jfree.report.flow.layoutprocessor.LayoutController;
import org.jfree.report.flow.layoutprocessor.SectionLayoutController;

/**
 * Creation-Date: 11.04.2007, 11:04:02
 *
 * @author Thomas Morgner
 */
public class OfficeDetailLayoutController extends SectionLayoutController
{
  public static final int STATE_PROCESS_VARIABLES = 2;
  public static final int STATE_PROCESS_NORMAL_FLOW = 3;

  private boolean waitForJoin;
  private int state;

  public OfficeDetailLayoutController()
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
    state = OfficeDetailLayoutController.STATE_PROCESS_VARIABLES;
  }

  /**
   * This method is called for each newly instantiated layout controller. The returned layout controller instance should
   * have a processing state of either 'OPEN' or 'FINISHING' depending on whether there is any content or any child
   * nodes to process.
   *
   * @param target the report target that receives generated events.
   * @return the new layout controller instance representing the new state.
   * @throws org.jfree.report.DataSourceException
   *          if there was a problem reading data from the datasource.
   * @throws org.jfree.report.ReportProcessingException
   *          if there was a general problem during the report processing.
   * @throws org.jfree.report.ReportDataFactoryException
   *          if a query failed.
   */
  protected LayoutController startElement(final ReportTarget target)
      throws DataSourceException, ReportProcessingException, ReportDataFactoryException
  {
    final FlowController fc = getFlowController();
    final GlobalMasterRow masterRow = fc.getMasterRow();
    final ReportDataRow reportDataRow = masterRow.getReportDataRow();
    final ReportData reportData = reportDataRow.getReportData();
    if (!reportData.isReadable())
    {
      reportData.isReadable();
      // If this report has no data, then do not print the detail section. The detail section
      // is the only section that behaves this way, and for now this is only done in the OO-implementation
      final SectionLayoutController derived = (SectionLayoutController) clone();
      derived.setProcessingState(ElementLayoutController.FINISHED);
      derived.setFlowController(fc);
      return derived;
    }

    if (state == OfficeDetailLayoutController.STATE_PROCESS_VARIABLES)
    {
      final VariablesDeclarationSection variables = new VariablesDeclarationSection();
      final OfficeDetailLayoutController controller = (OfficeDetailLayoutController) clone();
      controller.state = OfficeDetailLayoutController.STATE_PROCESS_NORMAL_FLOW;
      controller.waitForJoin = true;
      return processChild(controller, variables, fc);
    }

    return super.startElement(target);
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
      final OfficeDetailLayoutController derived = (OfficeDetailLayoutController) clone();
      derived.setProcessingState(ElementLayoutController.NOT_STARTED);
      derived.setFlowController(flowController);
      derived.waitForJoin = false;
      return derived;
    }
    return super.join(flowController);
  }

}
