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

import org.libreoffice.report.OfficeToken;
import org.libreoffice.report.pentaho.OfficeNamespaces;

import org.jfree.report.structure.Element;
import org.jfree.report.structure.Section;

/**
 * Represents an automatic or manual style definition.
 *
 * @since 02.03.2007
 */
public class OfficeStyle extends Section
{

    public OfficeStyle()
    {
        setNamespace(OfficeNamespaces.STYLE_NS);
        setType("style");
    }

    public String getStyleName()
    {
        return (String) getAttribute(OfficeNamespaces.STYLE_NS, "name");
    }

    public void setStyleName(final String name)
    {
        setAttribute(OfficeNamespaces.STYLE_NS, "name", name);
    }

    /**
     * A parent style name must be a common style (it cannot be an automatic
     * style) and has to exist. If no parent style is given, an implementation
     * specific default style is used.
     *
     * @return
     */
    public String getStyleParent()
    {
        return (String) getAttribute(OfficeNamespaces.STYLE_NS, "parent-style-name");
    }

    public void setStyleParent(final String parentName)
    {
        setAttribute(OfficeNamespaces.STYLE_NS, "parent-style-name", parentName);
    }

    public String getStyleFamily()
    {
        return (String) getAttribute(OfficeNamespaces.STYLE_NS, "family");
    }

    public void setStyleFamily(final String family)
    {
        setAttribute(OfficeNamespaces.STYLE_NS, "family", family);
    }

//  public String getMasterPageName()
//  {
//    return (String) getAttribute(OfficeNamespaces.STYLE_NS, "master-page-name");
//  }
    public Element getParagraphProperties()
    {
        return findFirstChild(OfficeNamespaces.STYLE_NS, "paragraph-properties");
    }

    public Element getTextProperties()
    {
        return findFirstChild(OfficeNamespaces.STYLE_NS, "text-properties");
    }

    public Element getTableRowProperties()
    {
        return findFirstChild(OfficeNamespaces.STYLE_NS, "table-row-properties");
    }

    public Element getTableProperties()
    {
        return findFirstChild(OfficeNamespaces.STYLE_NS, "table-properties");
    }

    public Element getTableColumnProperties()
    {
        return findFirstChild(OfficeNamespaces.STYLE_NS, "table-column-properties");
    }

    public Element getSectionProperties()
    {
        return findFirstChild(OfficeNamespaces.STYLE_NS, "section-properties");
    }

    public Element getTableCellProperties()
    {
        return findFirstChild(OfficeNamespaces.STYLE_NS, "table-cell-properties");
    }

    public Element getGraphicProperties()
    {
        return findFirstChild(OfficeNamespaces.STYLE_NS, OfficeToken.GRAPHIC_PROPERTIES);
    }
}
