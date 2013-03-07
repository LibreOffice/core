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

import org.libreoffice.report.pentaho.model.OfficeGroup;
import org.libreoffice.report.pentaho.model.OfficeGroupSection;

import org.jfree.layouting.util.AttributeMap;
import org.jfree.report.DataSourceException;
import org.jfree.report.JFreeReportInfo;
import org.jfree.report.ReportDataFactoryException;
import org.jfree.report.ReportProcessingException;
import org.jfree.report.flow.FlowController;
import org.jfree.report.flow.ReportTarget;
import org.jfree.report.flow.layoutprocessor.ElementLayoutController;
import org.jfree.report.flow.layoutprocessor.LayoutController;
import org.jfree.report.flow.layoutprocessor.SectionLayoutController;
import org.jfree.report.structure.Element;

/**
 * Todo: Document me!
 *
 * @since 15.03.2007
 */
public class OfficeGroupLayoutController extends SectionLayoutController
        implements OfficeRepeatingStructureLayoutController
{

    public static final int STATE_PROCESS_REPEATING_HEADER = 0;
    public static final int STATE_PROCESS_REPEATING_FOOTER = 1;
    public static final int STATE_PROCESS_NORMAL_FLOW = 3;
    private boolean waitForJoin;
    private int state;
    private VariablesCollection variablesCollection;
    private boolean repeatHeader;
    private boolean repeatFooter;

    public OfficeGroupLayoutController()
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
    public void initialize(final Object node,
            final FlowController flowController,
            final LayoutController parent)
            throws DataSourceException, ReportDataFactoryException,
            ReportProcessingException
    {
        super.initialize(node, flowController, parent);
        state = OfficeGroupLayoutController.STATE_PROCESS_REPEATING_HEADER;
        variablesCollection = new VariablesCollection(computeVariablesPrefix());


        final OfficeGroup group = (OfficeGroup) getElement();
        final OfficeGroupSection header = group.getHeader();
        repeatHeader = (header != null && header.isRepeatSection());

        final OfficeGroupSection footer = group.getFooter();
        repeatFooter = (footer != null && footer.isRepeatSection());
    }

    protected LayoutController processContent(final ReportTarget target)
            throws DataSourceException, ReportProcessingException,
            ReportDataFactoryException
    {
        if (state == OfficeGroupLayoutController.STATE_PROCESS_REPEATING_HEADER)
        {

            final OfficeGroupLayoutController controller =
                    (OfficeGroupLayoutController) clone();
            controller.state =
                    OfficeGroupLayoutController.STATE_PROCESS_REPEATING_FOOTER;

            if (!repeatHeader)
            {
                return controller;
            }

            final OfficeGroup group = (OfficeGroup) getElement();
            final OfficeGroupSection header = group.getHeader();
            controller.waitForJoin = true;
            return processChild(controller, header, getFlowController());
        }

        if (state == OfficeGroupLayoutController.STATE_PROCESS_REPEATING_FOOTER)
        {

            final OfficeGroupLayoutController controller =
                    (OfficeGroupLayoutController) clone();
            controller.state = OfficeGroupLayoutController.STATE_PROCESS_NORMAL_FLOW;

            if (!repeatFooter)
            {
                return controller;
            }

            final OfficeGroup group = (OfficeGroup) getElement();
            final OfficeGroupSection footer = group.getFooter();
            controller.waitForJoin = true;
            return processChild(controller, footer, getFlowController());
        }

        return super.processContent(target);
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
        if (waitForJoin)
        {
            final OfficeGroupLayoutController derived = (OfficeGroupLayoutController) clone();
            derived.setProcessingState(ElementLayoutController.OPENED);
            derived.setFlowController(flowController);
            derived.waitForJoin = false;
            return derived;
        }
        return super.join(flowController);
    }

    public boolean isNormalFlowProcessing()
    {
        return state == OfficeGroupLayoutController.STATE_PROCESS_NORMAL_FLOW;
    }

    private String computeVariablesPrefix()
    {
        int count = 0;
        LayoutController lc = this;
        while (lc != null)
        {
            if (lc instanceof OfficeGroupLayoutController)
            {
                count++;
            }
            lc = lc.getParent();
        }
        return "auto_group_" + count + "_";
    }

    public VariablesCollection getVariablesCollection()
    {
        return variablesCollection;
    }

    protected AttributeMap computeAttributes(final FlowController fc, final Element element, final ReportTarget target)
            throws DataSourceException
    {
        final AttributeMap map = new AttributeMap(super.computeAttributes(fc, element, target));
        final String value = String.valueOf(repeatHeader || repeatFooter);
        map.setAttribute(JFreeReportInfo.REPORT_NAMESPACE, "repeating-header-or-footer", value);
        map.makeReadOnly();
        return map;
    }
}
