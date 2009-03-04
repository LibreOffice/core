/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OfficeReport.java,v $
 * $Revision: 1.5 $
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
package com.sun.star.report.pentaho.model;

import org.jfree.report.structure.Element;
import org.jfree.report.structure.Node;

/**
 * A office report is an ordered section. It contains several root-level bands
 * which need to be processed in a given order.
 *
 * A report can have named expression attached. These expressions will be
 * computed at the global scope and will be available for all child elements.
 *
 * @author Thomas Morgner
 * @since 02.03.2007
 */
public class OfficeReport extends Element {

    private Node pageHeader;
    private Node pageFooter;
    private Node columnHeader;
    private Node columnFooter;
    private Node reportHeader;
    private Node reportFooter;
    private Node bodySection;
    private Node preBodySection;
    private Node postBodySection;

    public Node getPostBodySection() {
        return postBodySection;
    }

    public void setPostBodySection(final Node postBodySection) {
        this.postBodySection = postBodySection;
    }

    public Node getPreBodySection() {
        return preBodySection;
    }

    public void setPreBodySection(final Node preBodySection) {
        this.preBodySection = preBodySection;
    }

    public OfficeReport() {
    }

    public Node getPageHeader() {
        return pageHeader;
    }

    public void setPageHeader(final Node pageHeader) {
        this.pageHeader = pageHeader;
    }

    public Node getPageFooter() {
        return pageFooter;
    }

    public void setPageFooter(final Node pageFooter) {
        this.pageFooter = pageFooter;
    }

    public Node getColumnHeader() {
        return columnHeader;
    }

    public void setColumnHeader(final Node columnHeader) {
        this.columnHeader = columnHeader;
    }

    public Node getColumnFooter() {
        return columnFooter;
    }

    public void setColumnFooter(final Node columnFooter) {
        this.columnFooter = columnFooter;
    }

    public Node getReportHeader() {
        return reportHeader;
    }

    public void setReportHeader(final Node reportHeader) {
        this.reportHeader = reportHeader;
    }

    public Node getReportFooter() {
        return reportFooter;
    }

    public void setReportFooter(final Node reportFooter) {
        this.reportFooter = reportFooter;
    }

    public Node getBodySection() {
        return bodySection;
    }

    public void setBodySection(final Node bodySection) {
        this.bodySection = bodySection;
    }
}
