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
 * This layoutcontroller simply checks, whether the parent layout controller
 * is a OfficeGroupLayoutController and whether this layout controller is
 * processing the normal flow or an repeating section. If a repeating section
 * is being processed, an marker attribute is added to the element's call
 * to OutputProcessor.startElement() and OutputProcessor.endElement().
 *
 * @since 19.03.2007
 */
public class OfficeGroupSectionLayoutController extends SectionLayoutController
{

    public OfficeGroupSectionLayoutController()
    {
    }

    protected LayoutController startElement(final ReportTarget target)
            throws DataSourceException, ReportProcessingException, ReportDataFactoryException
    {
        final OfficeGroupSection section = (OfficeGroupSection) getElement();
        if (!section.isRepeatSection())
        {
            return super.startElement(target);
        }

        final LayoutController controller = getParent();
        if (!(controller instanceof OfficeGroupLayoutController))
        {
            return super.startElement(target);
        }
        final OfficeGroupLayoutController oglc = (OfficeGroupLayoutController) controller;
        if (!oglc.isNormalFlowProcessing())
        {
            return super.startElement(target);
        }

        // Skip the processing if the section is a repeating header or footer and we are processing the normal flow ..
        final ElementLayoutController clone = (ElementLayoutController) this.clone();
        clone.setProcessingState(ElementLayoutController.FINISHED);
        return clone;
    }

    protected AttributeMap computeAttributes(final FlowController fc,
            final Element element,
            final ReportTarget target)
            throws DataSourceException
    {
        final AttributeMap attrs = super.computeAttributes(fc, element, target);
        final LayoutController controller = getParent();
        if (!(controller instanceof OfficeGroupLayoutController))
        {
            return attrs;
        }
        final OfficeGroupLayoutController oglc = (OfficeGroupLayoutController) controller;
        if (oglc.isNormalFlowProcessing())
        {
            return attrs;
        }

        final AttributeMap retval = new AttributeMap(attrs);
        retval.setAttribute(JFreeReportInfo.REPORT_NAMESPACE, "repeated-section", OfficeToken.TRUE);
        retval.makeReadOnly();
        return retval;
    }
}
