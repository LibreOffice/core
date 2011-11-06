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

import com.sun.star.report.pentaho.model.FixedTextElement;

import org.jfree.report.DataSourceException;
import org.jfree.report.ReportDataFactoryException;
import org.jfree.report.ReportProcessingException;
import org.jfree.report.data.GlobalMasterRow;
import org.jfree.report.data.ReportDataRow;
import org.jfree.report.flow.FlowController;
import org.jfree.report.flow.ReportContext;
import org.jfree.report.flow.ReportTarget;
import org.jfree.report.flow.layoutprocessor.LayoutController;
import org.jfree.report.flow.layoutprocessor.LayoutControllerFactory;
import org.jfree.report.structure.Section;

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

    protected LayoutController delegateContentGeneration(final ReportTarget target)
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
