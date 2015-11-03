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
package org.libreoffice.report.pentaho.model;

import org.jfree.report.structure.Element;
import org.jfree.report.structure.Node;

/**
 * A office report is an ordered section. It contains several root-level bands
 * which need to be processed in a given order.
 *
 * A report can have named expression attached. These expressions will be
 * computed at the global scope and will be available for all child elements.
 *
 * @since 02.03.2007
 */
public class OfficeReport extends Element
{

    private Node pageHeader;
    private Node pageFooter;
    private Node reportHeader;
    private Node reportFooter;
    private Node bodySection;
    private Node preBodySection;
    private Node postBodySection;

    public Node getPostBodySection()
    {
        return postBodySection;
    }

    public void setPostBodySection(final Node postBodySection)
    {
        this.postBodySection = postBodySection;
    }

    public Node getPreBodySection()
    {
        return preBodySection;
    }

    public void setPreBodySection(final Node preBodySection)
    {
        this.preBodySection = preBodySection;
    }

    public Node getPageHeader()
    {
        return pageHeader;
    }

    public void setPageHeader(final Node pageHeader)
    {
        this.pageHeader = pageHeader;
    }

    public Node getPageFooter()
    {
        return pageFooter;
    }

    public void setPageFooter(final Node pageFooter)
    {
        this.pageFooter = pageFooter;
    }

    public Node getColumnHeader()
    {
        return null;
    }

    public Node getColumnFooter()
    {
        return null;
    }

    public Node getReportHeader()
    {
        return reportHeader;
    }

    public void setReportHeader(final Node reportHeader)
    {
        this.reportHeader = reportHeader;
    }

    public Node getReportFooter()
    {
        return reportFooter;
    }

    public void setReportFooter(final Node reportFooter)
    {
        this.reportFooter = reportFooter;
    }

    public Node getBodySection()
    {
        return bodySection;
    }

    public void setBodySection(final Node bodySection)
    {
        this.bodySection = bodySection;
    }
}
