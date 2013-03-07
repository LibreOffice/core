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
package org.libreoffice.report.pentaho.output.text;

import org.libreoffice.report.pentaho.styles.LengthCalculator;

import org.jfree.layouting.input.style.values.CSSNumericValue;

/**
 * Todo: Document me!
 *
 * @since 24.03.2007
 */
public class PageContext
{

    public static final int KEEP_TOGETHER_OFF = 0;
    public static final int KEEP_TOGETHER_GROUP = 1;
    public static final int KEEP_TOGETHER_FIRST_DETAIL = 2;
    private PageContext parent;
    private String header;
    private CSSNumericValue headerHeight;
    private String footer;
    private CSSNumericValue footerHeight;
    private int keepTogether;
    private Integer columnCount = null;
    private boolean sectionOpen;

    public PageContext()
    {
        this(null);
    }

    public PageContext(final PageContext parent)
    {
        this.parent = parent;
        if (parent != null)
        {
            this.keepTogether = parent.getKeepTogether();
        }
    }

    public int getActiveColumns()
    {
        PageContext pc = this;
        while (pc != null)
        {
            // TODO: IS this code correct? Why not columnCount = pc.getColumnCount(); ?
            if (columnCount != null)
            {
                return columnCount;
            }
            pc = pc.getParent();
        }
        return 1;
    }

    public void setColumnCount(final Integer columnCount)
    {
        this.columnCount = columnCount;
    }

    public Integer getColumnCount()
    {
        return columnCount;
    }

    public String getHeader()
    {
        return header;
    }

    public void setHeader(final String header, final CSSNumericValue height)
    {
        this.header = header;
        this.headerHeight = height;
    }

    public String getFooter()
    {
        return footer;
    }

    public CSSNumericValue getHeaderHeight()
    {
        return headerHeight;
    }

    public CSSNumericValue getFooterHeight()
    {
        return footerHeight;
    }

    public void setFooter(final String footer, final CSSNumericValue height)
    {
        this.footer = footer;
        this.footerHeight = height;
    }

    public int getKeepTogether()
    {
        return keepTogether;
    }

    public void setKeepTogether(final int keepTogether)
    {
        this.keepTogether = keepTogether;
    }

    public PageContext getParent()
    {
        return parent;
    }

    public CSSNumericValue getAllFooterSize()
    {
        if (parent == null)
        {
            return footerHeight;
        }

        final LengthCalculator lnc = new LengthCalculator();
        PageContext pc = this;
        while (pc != null)
        {
            lnc.add(pc.getFooterHeight());
            pc = pc.getParent();
        }
        return lnc.getResult();
    }

    public CSSNumericValue getAllHeaderSize()
    {
        if (parent == null)
        {
            return headerHeight;
        }

        final LengthCalculator lnc = new LengthCalculator();
        PageContext pc = this;
        while (pc != null)
        {
            lnc.add(pc.getHeaderHeight());
            pc = pc.getParent();
        }
        return lnc.getResult();
    }

    public String getPageFooterContent()
    {
        if (parent == null)
        {
            return getFooter();
        }

        final StringBuffer b = new StringBuffer();

        PageContext pc = this;
        while (pc != null)
        {
            final String footer_ = pc.getFooter();
            if (footer_ != null)
            {
                b.append(footer_);
            }
            pc = pc.getParent();
        }

        if (b.length() != 0)
        {
            return b.toString();
        }
        return null;
    }

    public String getPageHeaderContent()
    {
        if (parent == null)
        {
            return getHeader();
        }

        final StringBuffer b = new StringBuffer();
        b.append(parent.getPageHeaderContent());
        b.append(getHeader());

        if (b.length() != 0)
        {
            return b.toString();
        }
        return null;
    }

    public boolean isSectionOpen()
    {
        return sectionOpen;
    }

    public void setSectionOpen(final boolean sectionOpen)
    {
        this.sectionOpen = sectionOpen;
    }
}
