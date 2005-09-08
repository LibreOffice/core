/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _SpreadsheetViewSettings.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:39:49 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
