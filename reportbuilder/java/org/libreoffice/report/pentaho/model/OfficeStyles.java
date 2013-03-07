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

import java.io.Serializable;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.jfree.report.structure.Element;


/**
 * Holds one style type, either an automatic, common or master style. This is a
 * marker container that defines the nature of the styles contained within this
 * container. (Yeah, it is awkward, but that's how the document model describes
 * it.)
 *
 * The style family can be one of paragraph, text, section, table, table-column,
 * table-row, table-cell, table-page, chart, default, drawing-page, graphic,
 * presentation, control and ruby.
 *
 * @since 07.03.2007
 */
public class OfficeStyles extends Element
{

    public static class StyleKey implements Serializable
    {

        private static final long serialVersionUID = 4931878927362887477L;
        private final String family;
        private final String name;

        public StyleKey(final String family, final String name)
        {
            if (family == null)
            {
                throw new NullPointerException();
            }
            this.family = family;
            this.name = name;
        }

        public String getFamily()
        {
            return family;
        }

        public String getName()
        {
            return name;
        }

        public boolean equals(final Object obj)
        {
            if (this != obj)
            {
                if (obj == null || getClass() != obj.getClass())
                {
                    return false;
                }

                final StyleKey styleKey = (StyleKey) obj;

                if (!family.equals(styleKey.family) || (name != null ? !name.equals(styleKey.name) : styleKey.name != null))
                {
                    return false;
                }
            }
            return true;
        }

        public int hashCode()
        {
            int result = family.hashCode();
            result = 31 * result + (name != null ? name.hashCode() : 0);
            return result;
        }
    }
    private final Map<String,PageLayout> pageStyles;
    private final Map<String,DataStyle> dataStyles;
    private final Map<StyleKey,OfficeStyle> styles;
    private final List<Element> otherChildren;

    public OfficeStyles()
    {
        this.styles = new HashMap<StyleKey,OfficeStyle>();
        this.dataStyles = new HashMap<String,DataStyle>();
        this.pageStyles = new HashMap<String,PageLayout>();
        this.otherChildren = new ArrayList<Element>();
    }

    public OfficeStyle getStyle(final String family, final String name)
    {
        return styles.get(new StyleKey(family, name));
    }

    public void addStyle(final OfficeStyle style)
    {
        if (style == null)
        {
            throw new NullPointerException();
        }
        final String styleFamily = style.getStyleFamily();
        if (styleFamily == null)
        {
            throw new NullPointerException();
        }
        if (style.getStyleName() == null)
        {
            throw new NullPointerException();
        }
        styles.put(new StyleKey(styleFamily, style.getStyleName()), style);
    }

    public void addPageStyle(final PageLayout style)
    {
        pageStyles.put(style.getStyleName(), style);
    }

    public PageLayout getPageStyle(final String name)
    {
        return pageStyles.get(name);
    }

    public void addDataStyle(final DataStyle style)
    {
        dataStyles.put(style.getStyleName(), style);
    }

    public DataStyle getDataStyle(final String name)
    {
        return dataStyles.get(name);
    }

    public void addOtherNode(final Element node)
    {
        otherChildren.add(node);
    }

    public DataStyle[] getAllDataStyles()
    {
        return dataStyles.values().toArray(new DataStyle[dataStyles.size()]);
    }

    public PageLayout[] getAllPageStyles()
    {
        return pageStyles.values().toArray(new PageLayout[pageStyles.size()]);
    }

    public OfficeStyle[] getAllStyles()
    {
        return styles.values().toArray(new OfficeStyle[styles.size()]);
    }

    public Element[] getOtherStyles()
    {
        return otherChildren.toArray(new Element[otherChildren.size()]);
    }

    public boolean containsStyle(final String family, final String name)
    {
        return styles.containsKey(new StyleKey(family, name));
    }

    public boolean containsDataStyle(final String styleName)
    {
        return dataStyles.containsKey(styleName);
    }
}
