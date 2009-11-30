/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PageContext.java,v $
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
package com.sun.star.report.pentaho.output.text;

import org.jfree.layouting.input.style.values.CSSNumericValue;
import com.sun.star.report.pentaho.styles.LengthCalculator;

/**
 * Todo: Document me!
 *
 * @author Thomas Morgner
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
                return columnCount.intValue();
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
