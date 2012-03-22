/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/
package com.sun.star.report.pentaho.layoutprocessor;

import org.jfree.layouting.util.AttributeMap;
import org.jfree.report.DataSourceException;
import org.jfree.report.JFreeReportInfo;
import org.jfree.report.flow.FlowController;
import org.jfree.report.flow.ReportTarget;
import org.jfree.report.flow.layoutprocessor.SectionLayoutController;
import org.jfree.report.structure.Element;

/**
 * Todo: Document Me
 *
 * @author Thomas Morgner
 */
public class OfficePageSectionLayoutController extends SectionLayoutController
{

    public OfficePageSectionLayoutController()
    {
    }

    protected AttributeMap computeAttributes(final FlowController flowController, final Element element, final ReportTarget reportTarget) throws DataSourceException
    {
        final AttributeMap map = new AttributeMap(super.computeAttributes(flowController, element, reportTarget));
        map.setAttribute(JFreeReportInfo.REPORT_NAMESPACE, "role", "spreadsheet-section");
        map.makeReadOnly();
        return map;
    }
}
