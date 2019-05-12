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
