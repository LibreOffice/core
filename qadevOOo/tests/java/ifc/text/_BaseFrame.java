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

package ifc.text;

import lib.MultiPropertyTest;

/**
* Testing <code>com.sun.star.text.BaseFrame</code>
* service properties :
* <ul>
*  <li><code> AnchorPageNo</code></li>
*  <li><code> AnchorFrame</code></li>
*  <li><code> BackColor</code></li>
*  <li><code> BackGraphicURL</code></li>
*  <li><code> BackGraphicFilter</code></li>
*  <li><code> BackGraphicLocation</code></li>
*  <li><code> LeftBorder</code></li>
*  <li><code> RightBorder</code></li>
*  <li><code> TopBorder</code></li>
*  <li><code> BottomBorder</code></li>
*  <li><code> BorderDistance</code></li>
*  <li><code> LeftBorderDistance</code></li>
*  <li><code> RightBorderDistance</code></li>
*  <li><code> TopBorderDistance</code></li>
*  <li><code> BottomBorderDistance</code></li>
*  <li><code> BackTransparent</code></li>
*  <li><code> ContentProtected</code></li>
*  <li><code> FrameStyleName</code></li>
*  <li><code> LeftMargin</code></li>
*  <li><code> RightMargin</code></li>
*  <li><code> TopMargin</code></li>
*  <li><code> BottomMargin</code></li>
*  <li><code> Height</code></li>
*  <li><code> Width</code></li>
*  <li><code> RelativeHeight</code></li>
*  <li><code> RelativeWidth</code></li>
*  <li><code> IsSyncWidthToHeight</code></li>
*  <li><code> IsSyncHeightToWidth</code></li>
*  <li><code> HoriOrient</code></li>
*  <li><code> HoriOrientPosition</code></li>
*  <li><code> HoriOrientRelation</code></li>
*  <li><code> VertOrient</code></li>
*  <li><code> VertOrientPosition</code></li>
*  <li><code> VertOrientRelation</code></li>
*  <li><code> HyperLinkURL</code></li>
*  <li><code> HyperLinkTarget</code></li>
*  <li><code> HyperLinkName</code></li>
*  <li><code> Opaque</code></li>
*  <li><code> PageToggle</code></li>
*  <li><code> PositionProtected</code></li>
*  <li><code> Print</code></li>
*  <li><code> ShadowFormat</code></li>
*  <li><code> ServerMap</code></li>
*  <li><code> Size</code></li>
*  <li><code> SizeProtected</code></li>
*  <li><code> Surround</code></li>
*  <li><code> SurroundAnchorOnly</code></li>
*  <li><code> BackColorTransparency</code></li>
*  <li><code> BackColorRGB</code></li>
*  <li><code> BackGraphicTransparency</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.text.BaseFrame
*/
public class _BaseFrame extends MultiPropertyTest {
    public String str1 = "Graphics";
    public String str2 = "Watermark";

    /**
    * Redefined method returns value, that differs from property value.
    */
    protected PropertyTester StringTester = new PropertyTester() {
        @Override
        protected Object getNewValue(String propName, Object oldValue) {
            if (str1.equals(oldValue))
                return str2;
            else
                return str1;
        }
    };

    /**
    * This property can be VOID, and in case if it is so new
    * value must be defined.
    */
    public void _FrameStyleName() {
        log.println("Testing with custom Property tester") ;
        testProperty("FrameStyleName", StringTester) ;
    }

    /**
     * This property can be VOID, and in case if it is so new
     * value must be defined.
     */
    public void _BackColorTransparency() {
        testProperty("BackColorTransparency", Integer.valueOf(10), Integer.valueOf(50)) ;
    }

} //finish class _BaseFrame

