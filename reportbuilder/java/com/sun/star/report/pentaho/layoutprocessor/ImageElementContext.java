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


package com.sun.star.report.pentaho.layoutprocessor;

/**
 * Todo: Document me!
 *
 * @author Thomas Morgner
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
