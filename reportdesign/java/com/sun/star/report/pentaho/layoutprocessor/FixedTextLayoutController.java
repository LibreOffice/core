/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FixedTextLayoutController.java,v $
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
    if (reportDataRow.getCursor() == 0)
    {
      return true;
    }
    return false;
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
