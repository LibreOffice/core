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

import org.libreoffice.report.pentaho.model.OfficeReport;
import org.libreoffice.report.pentaho.model.VariablesDeclarationSection;

import org.jfree.report.DataSourceException;
import org.jfree.report.ReportDataFactoryException;
import org.jfree.report.ReportProcessingException;
import org.jfree.report.flow.FlowController;
import org.jfree.report.flow.ReportContext;
import org.jfree.report.flow.ReportTarget;
import org.jfree.report.flow.layoutprocessor.ElementLayoutController;
import org.jfree.report.flow.layoutprocessor.LayoutController;
import org.jfree.report.flow.layoutprocessor.LayoutControllerFactory;
import org.jfree.report.structure.Node;

/**
 * Todo: Document me!
 *
 * @since 06.03.2007
 */
public class OfficeReportLayoutController extends ElementLayoutController
        implements OfficeRepeatingStructureLayoutController
{

    private static final int STATE_NOT_STARTED = 0;
    private static final int STATE_TEMPLATES = 1;
    private static final int STATE_PAGE_HEADER_DONE = 2;
    private static final int STATE_PAGE_FOOTER_DONE = 3;
    private static final int STATE_SPREADSHEET_PAGE_HEADER_DONE = 4;
    private static final int STATE_SPREADSHEET_PAGE_FOOTER_DONE = 5;
    private static final int STATE_COLUMN_HEADER_DONE = 6;
    private static final int STATE_COLUMN_FOOTER_DONE = 7;
    private static final int STATE_INITIAL_VARIABLES_DONE = 8;
    private static final int STATE_REPORT_HEADER_DONE = 9;
    private static final int STATE_REPORT_BODY_DONE = 10;
    private static final int STATE_REPORT_FOOTER_VARIABLES = 11;
    private static final int STATE_REPORT_FOOTER_DONE = 12;
    private int state;
    private VariablesCollection variablesCollection;

    public OfficeReportLayoutController()
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
    public void initialize(final Object node, final FlowController flowController,
            final LayoutController parent)
            throws DataSourceException, ReportDataFactoryException,
            ReportProcessingException
    {
        super.initialize(node, flowController, parent);
        variablesCollection = new VariablesCollection("auto_report_");
    }

    /**
     * Processes any content in this element. This method is called when the
     * processing state is 'OPENED'. The returned layout controller will retain
     * the 'OPENED' state as long as there is more content available. Once all
     * content has been processed, the returned layout controller should carry a
     * 'FINISHED' state.
     *
     * @param target the report target that receives generated events.
     * @return the new layout controller instance representing the new state.
     *
     * @throws org.jfree.report.DataSourceException
     *          if there was a problem reading data from the datasource.
     * @throws org.jfree.report.ReportProcessingException
     *          if there was a general problem during the report processing.
     * @throws org.jfree.report.ReportDataFactoryException
     *          if a query failed.
     */
    protected LayoutController processContent(final ReportTarget target)
            throws DataSourceException, ReportProcessingException,
            ReportDataFactoryException
    {
        final OfficeReport or = (OfficeReport) getElement();

        switch (state)
        {
            case OfficeReportLayoutController.STATE_NOT_STARTED:
            {
                return delegateToTemplates(OfficeReportLayoutController.STATE_TEMPLATES);
            }
            case OfficeReportLayoutController.STATE_TEMPLATES:
            {
                return delegateSection(or.getPageHeader(),
                        OfficeReportLayoutController.STATE_PAGE_HEADER_DONE);
            }
            case OfficeReportLayoutController.STATE_PAGE_HEADER_DONE:
            {
                return delegateSpreadsheetSection(or.getPageHeader(),
                        OfficeReportLayoutController.STATE_SPREADSHEET_PAGE_HEADER_DONE);
            }
            case OfficeReportLayoutController.STATE_SPREADSHEET_PAGE_HEADER_DONE:
            {
                return delegateSection(or.getPageFooter(),
                        OfficeReportLayoutController.STATE_PAGE_FOOTER_DONE);
            }
            case OfficeReportLayoutController.STATE_PAGE_FOOTER_DONE:
            {
                return delegateSection(or.getColumnHeader(),
                        OfficeReportLayoutController.STATE_COLUMN_HEADER_DONE);
            }
            case OfficeReportLayoutController.STATE_COLUMN_HEADER_DONE:
            {
                return delegateSection(or.getColumnFooter(),
                        OfficeReportLayoutController.STATE_COLUMN_FOOTER_DONE);
            }
            case OfficeReportLayoutController.STATE_COLUMN_FOOTER_DONE:
            {
                return delegateSection(new VariablesDeclarationSection(),
                        OfficeReportLayoutController.STATE_INITIAL_VARIABLES_DONE);
            }
            case OfficeReportLayoutController.STATE_INITIAL_VARIABLES_DONE:
            {
                return delegateSection(or.getReportHeader(),
                        OfficeReportLayoutController.STATE_REPORT_HEADER_DONE);
            }
            case OfficeReportLayoutController.STATE_REPORT_HEADER_DONE:
            {
                return delegateSection(or.getBodySection(),
                        OfficeReportLayoutController.STATE_REPORT_BODY_DONE);
            }
            case OfficeReportLayoutController.STATE_REPORT_BODY_DONE:
            {
                return delegateSection(new VariablesDeclarationSection(),
                        OfficeReportLayoutController.STATE_REPORT_FOOTER_VARIABLES);
            }
            case OfficeReportLayoutController.STATE_REPORT_FOOTER_VARIABLES:
            {
                return delegateSection(or.getReportFooter(),
                        OfficeReportLayoutController.STATE_REPORT_FOOTER_DONE);
            }
            case OfficeReportLayoutController.STATE_REPORT_FOOTER_DONE:
            {
                return delegateSpreadsheetSection(or.getPageFooter(),
                        OfficeReportLayoutController.STATE_SPREADSHEET_PAGE_FOOTER_DONE);
            }
            case OfficeReportLayoutController.STATE_SPREADSHEET_PAGE_FOOTER_DONE:
            {
                final OfficeReportLayoutController olc = (OfficeReportLayoutController) clone();
                olc.setProcessingState(ElementLayoutController.FINISHING);
                return olc;
            }
            default:
            {
                throw new IllegalStateException("Invalid processing state encountered.");
            }
        }
    }

    private LayoutController delegateSpreadsheetSection(final Node node, final int nextState)
            throws DataSourceException, ReportProcessingException, ReportDataFactoryException
    {
        final OfficeReportLayoutController olc = (OfficeReportLayoutController) clone();
        olc.state = nextState;

        if (node == null)
        {
            return olc;
        }

        final OfficePageSectionLayoutController templateLc = new OfficePageSectionLayoutController();
        templateLc.initialize(node, getFlowController(), olc);
        return templateLc;
    }

    private LayoutController delegateToTemplates(final int nextState)
            throws ReportProcessingException, ReportDataFactoryException,
            DataSourceException
    {
        final OfficeReportLayoutController olc = (OfficeReportLayoutController) clone();
        olc.state = nextState;

        final OfficeTableTemplateLayoutController templateLc = new OfficeTableTemplateLayoutController();
        templateLc.initialize(getElement(), getFlowController(), olc);
        return templateLc;

    }

    private LayoutController delegateSection(final Node n, final int nextState)
            throws ReportProcessingException, ReportDataFactoryException,
            DataSourceException
    {
        final OfficeReportLayoutController olc = (OfficeReportLayoutController) clone();
        olc.state = nextState;
        if (n == null)
        {
            return olc;
        }

        final FlowController flowController = getFlowController();
        final ReportContext reportContext = flowController.getReportContext();
        final LayoutControllerFactory layoutControllerFactory =
                reportContext.getLayoutControllerFactory();
        return layoutControllerFactory.create(flowController, n, olc);

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
        final OfficeReportLayoutController derived = (OfficeReportLayoutController) clone();
        derived.setFlowController(flowController);
        return derived;
    }

    public boolean isNormalFlowProcessing()
    {
        return state != OfficeReportLayoutController.STATE_PAGE_HEADER_DONE && state != OfficeReportLayoutController.STATE_PAGE_FOOTER_DONE;
    }

    public VariablesCollection getVariablesCollection()
    {
        return variablesCollection;
    }
}
