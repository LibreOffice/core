/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OfficeGroupSectionLayoutController.java,v $
 * $Revision: 1.5 $
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

import org.jfree.report.flow.layoutprocessor.SectionLayoutController;
import org.jfree.report.flow.layoutprocessor.LayoutController;
import org.jfree.report.flow.layoutprocessor.ElementLayoutController;
import org.jfree.report.flow.FlowController;
import org.jfree.report.flow.ReportTarget;
import org.jfree.report.structure.Element;
import org.jfree.report.DataSourceException;
import org.jfree.report.ReportProcessingException;
import org.jfree.report.ReportDataFactoryException;
import org.jfree.layouting.util.AttributeMap;
import com.sun.star.report.OfficeToken;
import com.sun.star.report.pentaho.model.OfficeGroupSection;
import org.jfree.report.JFreeReportInfo;

/**
 * This layoutcontroller simply checks, whether the parent layout controller
 * is a OfficeGroupLayoutController and whether this layout controller is
 * processing the normal flow or an repeating section. If a repeating section
 * is being processed, an marker attribute is added to the element's call
 * to OutputProcessor.startElement() and OutputProcessor.endElement().
 *
 * @author Thomas Morgner
 * @since 19.03.2007
 */
public class OfficeGroupSectionLayoutController extends SectionLayoutController
{

  public OfficeGroupSectionLayoutController()
  {
  }

  protected LayoutController startElement(final ReportTarget target)
      throws DataSourceException, ReportProcessingException, ReportDataFactoryException
  {
    final OfficeGroupSection section = (OfficeGroupSection) getElement();
    if (!section.isRepeatSection())
    {
      return super.startElement(target);
    }

    final LayoutController controller = getParent();
    if (!(controller instanceof OfficeGroupLayoutController))
    {
      return super.startElement(target);
    }
    final OfficeGroupLayoutController oglc = (OfficeGroupLayoutController) controller;
    if (!oglc.isNormalFlowProcessing())
    {
      return super.startElement(target);
    }

    // Skip the processing if the section is a repeating header or footer and we are processing the normal flow ..
    final ElementLayoutController clone = (ElementLayoutController) this.clone();
    clone.setProcessingState(ElementLayoutController.FINISHED);
    return clone;
  }

  protected AttributeMap computeAttributes(final FlowController fc,
                                           final Element element,
                                           final ReportTarget target)
      throws DataSourceException
  {
    final AttributeMap attrs = super.computeAttributes(fc, element, target);
    final LayoutController controller = getParent();
    if (!(controller instanceof OfficeGroupLayoutController))
    {
      return attrs;
    }
    final OfficeGroupLayoutController oglc = (OfficeGroupLayoutController) controller;
    if (oglc.isNormalFlowProcessing())
    {
      return attrs;
    }

    final AttributeMap retval = new AttributeMap(attrs);
    retval.setAttribute(JFreeReportInfo.REPORT_NAMESPACE, "repeated-section", OfficeToken.TRUE);
    retval.makeReadOnly();
    return retval;
  }


}
