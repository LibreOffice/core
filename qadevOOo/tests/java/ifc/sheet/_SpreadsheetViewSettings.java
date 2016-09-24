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
        testProperty("ZoomType", Short.valueOf(DocumentZoomType.PAGE_WIDTH),
                     Short.valueOf(DocumentZoomType.BY_VALUE));
    }

    /**
    * Forces environment recreation.
    */
    @Override
    protected void after() {
        disposeEnvironment();
    }
} // finish class _SpreadsheetViewSettings
