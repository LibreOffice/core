/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OfficeTableLayoutController.java,v $
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
import com.sun.star.report.OfficeToken;
import org.jfree.report.JFreeReportInfo;

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
        final AttributeMap attributeMap = new AttributeMap( super.computeAttributes(fc, element, target) );
        final Section s = (Section) element;
        int rowCount = 0;
        final Node[] nodeArray = s.getNodeArray();
        for (int i = 0; i < nodeArray.length; i++)
        {
            final Node node = nodeArray[i];
            if (node instanceof Element)
            {
                final Element child = (Element) node;
                if (OfficeNamespaces.TABLE_NS.equals(child.getNamespace()) &&
                        OfficeToken.TABLE_ROW.equals(child.getType()))
                {
                    rowCount += 1;
                }
            }
        }

        attributeMap.setAttribute(JFreeReportInfo.REPORT_NAMESPACE, "table-row-count", IntegerCache.getInteger(rowCount));
        attributeMap.makeReadOnly();
        return attributeMap;
    }
}
