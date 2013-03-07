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

import org.jfree.report.structure.Section;

/**
 * A page layout describes the physical properties of a page. It is equal to
 * an @page rule in CSS.
 *
 * @since 13.03.2007
 */
public class PageLayout extends Section
{

    public PageLayout()
    {
        setNamespace(OfficeNamespaces.STYLE_NS);
        setType("page-layout");
    }

    public String getStyleName()
    {
        return (String) getAttribute(OfficeNamespaces.STYLE_NS, "name");
    }

    public void setStyleName(final String name)
    {
        setAttribute(OfficeNamespaces.STYLE_NS, "name", name);
    }

    public Section getHeaderStyle()
    {
        return (Section) findFirstChild(OfficeNamespaces.STYLE_NS, "header-style");
    }

    public Section getFooterStyle()
    {
        return (Section) findFirstChild(OfficeNamespaces.STYLE_NS, "footer-style");
    }
}
