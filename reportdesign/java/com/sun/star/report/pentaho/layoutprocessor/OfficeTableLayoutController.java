/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OfficeTableLayoutController.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:06 $
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
import org.jfree.report.flow.FlowController;
import org.jfree.report.flow.ReportTarget;
import org.jfree.report.structure.Element;
import org.jfree.report.structure.Section;
import org.jfree.report.structure.Node;
import org.jfree.report.DataSourceException;
import org.jfree.report.util.IntegerCache;
import org.jfree.layouting.util.AttributeMap;
import com.sun.star.report.pentaho.OfficeNamespaces;

/**
 * Creation-Date: 24.04.2007, 14:40:20
 *
 * @author Thomas Morgner
 */
public class OfficeTableLayoutController extends SectionLayoutController
{
  public OfficeTableLayoutController()
  {
  }

  protected AttributeMap computeAttributes(final FlowController fc, final Element element, final ReportTarget target)
      throws DataSourceException
  {
    final AttributeMap attributeMap = super.computeAttributes(fc, element, target);
    final Section s = (Section) element;
    int rowCount = 0;
    final Node[] nodeArray = s.getNodeArray();
    for (int i = 0; i < nodeArray.length; i++)
    {
      final Node node = nodeArray[i];
      if (node instanceof Element == false)
      {
        continue;
      }
      final Element child = (Element) node;
      if (OfficeNamespaces.TABLE_NS.equals(child.getNamespace()) &&
          "table-row".equals(child.getType()))
      {
        rowCount += 1;
      }
    }
    attributeMap.setAttribute(OfficeNamespaces.INTERNAL_NS, "table-row-count", IntegerCache.getInteger(rowCount));
    return attributeMap;
  }
}
