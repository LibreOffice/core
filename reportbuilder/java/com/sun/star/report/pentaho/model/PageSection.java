/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PageSection.java,v $
 * $Revision: 1.4 $
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

import org.jfree.report.structure.Section;
import org.jfree.layouting.util.AttributeMap;
import com.sun.star.report.pentaho.OfficeNamespaces;

/**
 * This represents either a page header or page footer.
 *
 * @author Thomas Morgner
 * @since 02.03.2007
 */
public class PageSection extends Section
{

    private static final String NOT_WITH_REPORT_HEADER_NOR_FOOTER = "not-with-report-header-nor-footer";

    public PageSection()
    {
    }

    public static boolean isPrintWithReportHeader(final AttributeMap attrs)
    {
        final String pagePrintOption = (String) attrs.getAttribute(OfficeNamespaces.OOREPORT_NS, "page-print-option");
        return !("not-with-report-header".equals(pagePrintOption) || NOT_WITH_REPORT_HEADER_NOR_FOOTER.equals(pagePrintOption));
    }

    public static boolean isPrintWithReportFooter(final AttributeMap attrs)
    {
        final String pagePrintOption = (String) attrs.getAttribute(OfficeNamespaces.OOREPORT_NS, "page-print-option");
        return !("not-with-report-footer".equals(pagePrintOption) || NOT_WITH_REPORT_HEADER_NOR_FOOTER.equals(pagePrintOption));
    }
}
