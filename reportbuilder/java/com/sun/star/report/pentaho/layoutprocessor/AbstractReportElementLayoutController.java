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

package com.sun.star.report.pentaho.layoutprocessor;

import com.sun.star.report.pentaho.model.OfficeGroupSection;
import com.sun.star.report.pentaho.model.ReportElement;

import org.jfree.report.DataFlags;
import org.jfree.report.DataRow;
import org.jfree.report.DataSourceException;
import org.jfree.report.ReportDataFactoryException;
import org.jfree.report.ReportProcessingException;
import org.jfree.report.expressions.Expression;
import org.jfree.report.flow.FlowController;
import org.jfree.report.flow.ReportTarget;
import org.jfree.report.flow.layoutprocessor.AbstractLayoutController;
import org.jfree.report.flow.layoutprocessor.LayoutController;
import org.jfree.report.flow.layoutprocessor.LayoutControllerUtil;
import org.jfree.report.flow.layoutprocessor.SectionLayoutController;
import org.jfree.report.structure.Element;
import org.jfree.report.structure.Group;

import org.pentaho.reporting.libraries.formula.lvalues.ContextLookup;
import org.pentaho.reporting.libraries.formula.lvalues.LValue;

/**
 * Todo: Document me!
 *
 * @since 05.03.2007
 * @noinspection CloneableClassWithoutClone
 */
public abstract class AbstractReportElementLayoutController
        extends AbstractLayoutController
{

    public static final int NOT_STARTED = 0;
    public static final int FINISHED = 2;
    private int state;

    protected AbstractReportElementLayoutController()
    {
    }

    /**
     * Advances the processing position.
     *
     * @param target the report target that receives generated events.
     * @return the new layout controller instance representing the new state.
     *
     * @throws org.jfree.report.DataSourceException        if there was a problem reading data from
     *                                    the datasource.
     * @throws org.jfree.report.ReportProcessingException  if there was a general problem during
     *                                    the report processing.
     * @throws org.jfree.report.ReportDataFactoryException if a query failed.
     */
    public LayoutController advance(final ReportTarget target)
            throws DataSourceException, ReportDataFactoryException,
            ReportProcessingException
    {
        if (state != AbstractReportElementLayoutController.NOT_STARTED)
        {
            throw new IllegalStateException();
        }

        if (FormatValueUtility.shouldPrint(this, (ReportElement)getNode()))
        {
            // delegate to the handler ..
            return delegateContentGeneration(target);
        }
        else
        {
            // There is no printable content at all. Set the state to FINISHED
            return join(getFlowController());
        }
    }

    public abstract boolean isValueChanged();

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
            throws DataSourceException, ReportDataFactoryException,
            ReportProcessingException
    {
        final AbstractReportElementLayoutController alc =
                (AbstractReportElementLayoutController) clone();
        alc.state = AbstractReportElementLayoutController.FINISHED;
        return alc;
    }

    protected abstract LayoutController delegateContentGeneration(final ReportTarget target)
            throws ReportProcessingException, ReportDataFactoryException,
            DataSourceException;

    /**
     * Checks, whether the layout controller would be advanceable. If this method
     * returns true, it is generally safe to call the 'advance()' method.
     *
     * @return true, if the layout controller is advanceable, false otherwise.
     */
    public boolean isAdvanceable()
    {
        return state != AbstractReportElementLayoutController.FINISHED;
    }

    public int getState()
    {
        return state;
    }

    protected void setState(final int state)
    {
        this.state = state;
    }
}
