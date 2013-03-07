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
 * A master page. A master page can contain header and footer and is linked
 * to a page-layout.
 *
 * @since 13.03.2007
 */
public class OfficeMasterPage extends Section
{

    public OfficeMasterPage()
    {
    }

    public String getStyleName()
    {
        return (String) getAttribute(OfficeNamespaces.STYLE_NS, "name");
    }

    public void setStyleName(final String name)
    {
        setAttribute(OfficeNamespaces.STYLE_NS, "name", name);
    }

    public String getPageLayout()
    {
        return (String) getAttribute(OfficeNamespaces.STYLE_NS, "page-layout-name");
    }

    public void setPageLayout(final String name)
    {
        setAttribute(OfficeNamespaces.STYLE_NS, "page-layout-name", name);
    }
}
