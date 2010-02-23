/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
package ifc.sheet;

import com.sun.star.view.DocumentZoomType;

import lib.MultiPropertyTest;


/**
* Testing <code>com.sun.star.sheet.SpreadsheetViewSettings</code>
* service properties :
* <ul>
*  <li><code> ShowFormulas</code></li>
*  <li><code> ShowZeroValues</code></li>
*  <li><code> IsValueHighlightingEnabled</code></li>
*  <li><code> ShowNotes</code></li>
*  <li><code> HasVerticalScrollBar</code></li>
*  <li><code> HasHorizontalScrollBar</code></li>
*  <li><code> HasSheetTabs</code></li>
*  <li><code> IsOutlineSymbolsSet</code></li>
*  <li><code> HasColumnRowHeaders</code></li>
*  <li><code> ShowGrid</code></li>
*  <li><code> GridColor</code></li>
*  <li><code> ShowHelpLines</code></li>
*  <li><code> ShowAnchor</code></li>
*  <li><code> ShowPageBreaks</code></li>
*  <li><code> SolidHandles</code></li>
*  <li><code> ShowObjects</code></li>
*  <li><code> ShowCharts</code></li>
*  <li><code> ShowDrawing</code></li>
*  <li><code> HideSpellMarks</code></li>
*  <li><code> ZoomType</code></li>
*  <li><code> ZoomValue</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.sheet.SpreadsheetViewSettings
*/
public class _SpreadsheetViewSettings extends MultiPropertyTest {
    public void _ZoomType() {
        testProperty("ZoomType", new Short(DocumentZoomType.PAGE_WIDTH),
                     new Short(DocumentZoomType.BY_VALUE));
    }

    /**
    * Forces environment recreation.
    */
    protected void after() {
        disposeEnvironment();
    }
} // finish class _SpreadsheetViewSettings
