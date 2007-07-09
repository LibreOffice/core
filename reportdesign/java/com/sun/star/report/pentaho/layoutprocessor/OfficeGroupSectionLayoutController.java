/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OfficeGroupSectionLayoutController.java,v $
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

import org.jfree.report.flow.layoutprocessor.SectionLayoutController;
import org.jfree.report.flow.layoutprocessor.LayoutController;
import org.jfree.report.flow.FlowController;
import org.jfree.report.flow.ReportTarget;
import org.jfree.report.structure.Element;
import org.jfree.report.DataSourceException;
import org.jfree.layouting.util.AttributeMap;
import com.sun.star.report.pentaho.OfficeNamespaces;

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


  protected AttributeMap computeAttributes(final FlowController fc,
                                           final Element element,
                                           final ReportTarget target)
      throws DataSourceException
  {
    final AttributeMap attrs = super.computeAttributes(fc, element, target);
    final LayoutController controller = getParent();
    if (controller instanceof OfficeGroupLayoutController == false)
    {
      return attrs;
    }
    final OfficeGroupLayoutController oglc =
        (OfficeGroupLayoutController) controller;
    if (oglc.isNormalFlowProcessing())
    {
      return attrs;
    }

    attrs.setAttribute(OfficeNamespaces.INTERNAL_NS, "repeated-section", "true");
    return attrs;
  }
}
