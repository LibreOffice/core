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

package org.libreoffice.report.pentaho.layoutprocessor;

/**
 * Todo: Document me!
 *
 * @since 30.03.2007
 */
public class ImageElementContext
{

    private final int colSpan;
    private final int rowSpan;
    private String[] rowStyles;
    private String[] colStyles;

    public ImageElementContext(final int colSpan, final int rowSpan)
    {
        this.colSpan = colSpan;
        this.rowSpan = rowSpan;
        this.colStyles = new String[colSpan];
        this.rowStyles = new String[rowSpan];
    }

    public int getColSpan()
    {
        return colSpan;
    }

    public int getRowSpan()
    {
        return rowSpan;
    }

    public String[] getRowStyles()
    {
        return rowStyles;
    }

    public String[] getColStyles()
    {
        return colStyles;
    }

    public void setRowStyle(final int pos, final String styleName)
    {
        rowStyles[pos] = styleName;
    }

    public void setColStyle(final int pos, final String styleName)
    {
        colStyles[pos] = styleName;
    }

    public String getRowStyle(final int pos)
    {
        return rowStyles[pos];
    }

    public String getColStyle(final int pos)
    {
        return colStyles[pos];
    }
}
