/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package org.libreoffice.report.pentaho.layoutprocessor;

import org.libreoffice.report.OfficeToken;
import org.libreoffice.report.pentaho.OfficeNamespaces;

import org.jfree.layouting.util.AttributeMap;
import org.jfree.report.DataSourceException;
import org.jfree.report.JFreeReportInfo;
import org.jfree.report.flow.FlowController;
import org.jfree.report.flow.ReportTarget;
import org.jfree.report.flow.layoutprocessor.SectionLayoutController;
import org.jfree.report.structure.Element;
import org.jfree.report.structure.Node;
import org.jfree.report.structure.Section;
import org.jfree.report.util.IntegerCache;

/**
 * Creation-Date: 24.04.2007, 14:40:20
 *
 */
public class OfficeTableLayoutController extends SectionLayoutController
{

    public OfficeTableLayoutController()
    {
    }

    protected AttributeMap computeAttributes(final FlowController fc, final Element element, final ReportTarget target)
            throws DataSourceException
    {
        final AttributeMap attributeMap = new AttributeMap(super.computeAttributes(fc, element, target));
        final Section s = (Section) element;
        int rowCount = 0;
        final Node[] nodeArray = s.getNodeArray();
        for (int i = 0; i < nodeArray.length; i++)
        {
            final Node node = nodeArray[i];
            if (node instanceof Element)
            {
                final Element child = (Element) node;
                if (OfficeNamespaces.TABLE_NS.equals(child.getNamespace()) && OfficeToken.TABLE_ROW.equals(child.getType()))
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
