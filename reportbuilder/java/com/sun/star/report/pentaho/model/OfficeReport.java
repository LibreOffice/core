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
public class OfficeReport extends Element
{

    private Node pageHeader;
    private Node pageFooter;
    private Node columnHeader;
    private Node columnFooter;
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

    public OfficeReport()
    {
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
        return columnHeader;
    }

    public void setColumnHeader(final Node columnHeader)
    {
        this.columnHeader = columnHeader;
    }

    public Node getColumnFooter()
    {
        return columnFooter;
    }

    public void setColumnFooter(final Node columnFooter)
    {
        this.columnFooter = columnFooter;
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
