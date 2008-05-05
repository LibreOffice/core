/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: FixedTextLayoutController.java,v $
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

import org.jfree.report.flow.layoutprocessor.LayoutController;
import org.jfree.report.flow.layoutprocessor.LayoutControllerFactory;
import org.jfree.report.flow.ReportTarget;
import org.jfree.report.flow.FlowController;
import org.jfree.report.flow.ReportContext;
import org.jfree.report.data.GlobalMasterRow;
import org.jfree.report.data.ReportDataRow;
import org.jfree.report.structure.Section;
import org.jfree.report.ReportProcessingException;
import org.jfree.report.ReportDataFactoryException;
import org.jfree.report.DataSourceException;
import com.sun.star.report.pentaho.model.FixedTextElement;

/**
 * Processes a fixed-text element of the OpenOffice reporting specifciation.
 * The element itself contains a single paragraph which contains the content.
 * After checking, whether this element should be printed, this layout
 * controller simply delegates the dirty work to a suitable handler.
 *
 * @author Thomas Morgner
 * @noinspection CloneableClassWithoutClone
 * @since 05.03.2007
 */
public class FixedTextLayoutController
    extends AbstractReportElementLayoutController
{

  public FixedTextLayoutController()
  {
  }


  protected boolean isValueChanged()
  {
    final FlowController controller = getFlowController();
    final GlobalMasterRow masterRow = controller.getMasterRow();
    final ReportDataRow reportDataRow = masterRow.getReportDataRow();
    return reportDataRow.getCursor() == 0;
  }

  protected LayoutController delegateContentGeneration
      (final ReportTarget target)
      throws ReportProcessingException, ReportDataFactoryException,
      DataSourceException
  {
    final FixedTextElement fte = (FixedTextElement) getNode();
    final Section content = fte.getContent();

    final FlowController flowController = getFlowController();
    final ReportContext reportContext = flowController.getReportContext();
    final LayoutControllerFactory layoutControllerFactory =
        reportContext.getLayoutControllerFactory();

    final FixedTextLayoutController flc = (FixedTextLayoutController) clone();
    flc.setState(AbstractReportElementLayoutController.FINISHED);
    return layoutControllerFactory.create(flowController, content, flc);
  }
}
