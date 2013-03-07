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

import org.libreoffice.report.pentaho.model.OfficeGroupSection;
import org.libreoffice.report.pentaho.model.VariablesDeclarationSection;

import org.jfree.layouting.util.AttributeMap;
import org.jfree.report.DataSourceException;
import org.jfree.report.JFreeReportInfo;
import org.jfree.report.ReportDataFactoryException;
import org.jfree.report.ReportProcessingException;
import org.jfree.report.expressions.Expression;
import org.jfree.report.flow.FlowController;
import org.jfree.report.flow.ReportContext;
import org.jfree.report.flow.ReportTarget;
import org.jfree.report.flow.layoutprocessor.ElementLayoutController;
import org.jfree.report.flow.layoutprocessor.LayoutController;
import org.jfree.report.flow.layoutprocessor.LayoutControllerFactory;
import org.jfree.report.flow.layoutprocessor.LayoutControllerUtil;
import org.jfree.report.flow.layoutprocessor.SectionLayoutController;
import org.jfree.report.structure.Element;
import org.jfree.report.structure.Node;

/**
 * Creation-Date: 25.07.2007, 14:50:45
 *
 */
public class OfficeGroupInstanceSectionLayoutController extends SectionLayoutController
{

    public static final int STATE_PROCESS_VARIABLES = 2;
    public static final int STATE_PROCESS_NORMAL_FLOW = 3;
    private int state;
    private boolean waitForJoin;

    public OfficeGroupInstanceSectionLayoutController()
    {
    }

    public void initialize(final Object node, final FlowController flowController, final LayoutController parent)
            throws DataSourceException, ReportDataFactoryException, ReportProcessingException
    {
        super.initialize(node, flowController, parent);
        state = STATE_PROCESS_VARIABLES;
    }

    protected LayoutController processContent(final ReportTarget target)
            throws DataSourceException, ReportProcessingException, ReportDataFactoryException
    {
        if (state == OfficeGroupInstanceSectionLayoutController.STATE_PROCESS_VARIABLES)
        {
            // todo: Fill the variables section with something sensible ..
            final VariablesDeclarationSection variables = new VariablesDeclarationSection();
            final OfficeGroupInstanceSectionLayoutController controller =
                    (OfficeGroupInstanceSectionLayoutController) clone();
            controller.state =
                    OfficeGroupLayoutController.STATE_PROCESS_NORMAL_FLOW;
            controller.waitForJoin = true;
            return processChild(controller, variables, getFlowController());
        }
        return super.processContent(target);
    }

    // isDisplayable is private in version 0.9.1, so until the upgrade we keep this copy of the method
    // todo: Delete it unce the sun-cvs contains version 0.9.2.
    protected LayoutController processChild(final SectionLayoutController derived,
            final Node node,
            final FlowController flowController)
            throws DataSourceException, ReportProcessingException,
            ReportDataFactoryException
    {
        final ReportContext reportContext = flowController.getReportContext();
        final LayoutControllerFactory layoutControllerFactory = reportContext.getLayoutControllerFactory();
        if (isDisplayable(node))
        {
            derived.setProcessingState(ElementLayoutController.WAITING_FOR_JOIN);
            return layoutControllerFactory.create(flowController, node, derived);
        }
        else
        {
            derived.setProcessingState(ElementLayoutController.WAITING_FOR_JOIN);
            final LayoutController childLc = layoutControllerFactory.create(flowController, node, derived);
            return LayoutControllerUtil.skipInvisibleElement(childLc);
        }
    }

    protected boolean isDisplayable(final Node node) throws DataSourceException
    {
        if (!(node instanceof OfficeGroupSection))
        {
            return _isDisplayable(node);
        }

        final OfficeGroupSection section = (OfficeGroupSection) node;
        return !section.isRepeatSection() && _isDisplayable(node);
    }

    protected boolean _isDisplayable(final Node node)
            throws DataSourceException
    {
        // temp method until the pending upgrade to 0.9.2. Later we just call super.isDisplayable(..) instead.
        if (!node.isEnabled())
        {
            return false;
        }

        final Expression expression = node.getDisplayCondition();
        if (expression == null)
        {
            return true;
        }

        final Object result = LayoutControllerUtil.evaluateExpression(getFlowController(), node, expression);
        return Boolean.TRUE.equals(result);
    }

    protected void resetSectionForRepeat()
    {
        super.resetSectionForRepeat();
        state = STATE_PROCESS_VARIABLES;
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
            final OfficeGroupInstanceSectionLayoutController derived = (OfficeGroupInstanceSectionLayoutController) clone();
            derived.setProcessingState(ElementLayoutController.OPENED);
            derived.setFlowController(flowController);
            derived.waitForJoin = false;
            return derived;
        }
        return super.join(flowController);
    }

    protected AttributeMap computeAttributes(final FlowController fc, final Element element, final ReportTarget target)
            throws DataSourceException
    {
        final AttributeMap map = new AttributeMap(super.computeAttributes(fc, element, target));
        map.setAttribute(JFreeReportInfo.REPORT_NAMESPACE, "iteration-count", getIterationCount());
        map.makeReadOnly();
        return map;
    }
}
