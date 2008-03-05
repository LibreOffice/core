/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OfficeReport.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:34:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *    Copyright 2007 by Pentaho Corporation
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

    public void setPostBodySection(Node postBodySection) {
        this.postBodySection = postBodySection;
    }

    public Node getPreBodySection() {
        return preBodySection;
    }

    public void setPreBodySection(Node preBodySection) {
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
