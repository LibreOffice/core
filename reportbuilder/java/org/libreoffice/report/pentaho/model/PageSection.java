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

import org.libreoffice.report.pentaho.OfficeNamespaces;

import org.jfree.layouting.util.AttributeMap;
import org.jfree.report.structure.Section;

/**
 * This represents either a page header or page footer.
 *
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
