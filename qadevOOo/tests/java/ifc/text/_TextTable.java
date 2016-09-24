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
 * Testing <code>com.sun.star.text.TextTable</code>
 * service properties :
 * <ul>
 *  <li><code> BreakType</code></li>
 *  <li><code> LeftMargin</code></li>
 *  <li><code> RightMargin</code></li>
 *  <li><code> HoriOrient</code></li>
 *  <li><code> KeepTogether</code></li>
 *  <li><code> Split</code></li>
 *  <li><code> PageNumberOffset</code></li>
 *  <li><code> PageDescName</code></li>
 *  <li><code> RelativeWidth</code></li>
 *  <li><code> IsWidthRelative</code></li>
 *  <li><code> RepeatHeadline</code></li>
 *  <li><code> ShadowFormat</code></li>
 *  <li><code> TopMargin</code></li>
 *  <li><code> BottomMargin</code></li>
 *  <li><code> BackTransparent</code></li>
 *  <li><code> Width</code></li>
 *  <li><code> ChartRowAsLabel</code></li>
 *  <li><code> ChartColumnAsLabel</code></li>
 *  <li><code> TableBorder</code></li>
 *  <li><code> TableColumnSeparators</code></li>
 *  <li><code> TableColumnRelativeSum</code></li>
 *  <li><code> BackColor</code></li>
 *  <li><code> BackGraphicURL</code></li>
 *  <li><code> BackGraphicFilter</code></li>
 *  <li><code> BackGraphicLocation</code></li>
 * </ul> <p>
 * Properties testing is automated by <code>lib.MultiPropertyTest</code>.
 * @see com.sun.star.text.TextTable
 */
public class _TextTable extends MultiPropertyTest {

    /**
     * This property accepts only restricted range of values.
     * The property value is switched between '' and 'Standard'
     * strings.
     */
    public void _PageDescName() {
        testProperty("PageDescName", "Endnote", "Standard");
    }

    /**
     * For setting this property, 'HoriOrient' property must be
     * set to non-automatic.
     */
    public void _Width() {
        Short align = Short.valueOf(com.sun.star.text.HoriOrientation.CENTER);
        try {
          oObj.setPropertyValue("HoriOrient",align);
        }
        catch (com.sun.star.lang.IllegalArgumentException ex) {}
        catch (com.sun.star.lang.WrappedTargetException ex) {}
        catch (com.sun.star.beans.PropertyVetoException ex) {}
        catch (com.sun.star.beans.UnknownPropertyException ex) {}

        testProperty("Width");
    }

    /**
     * For setting this property, 'HoriOrient' property must be
     * set to non-automatic.
     */
    public void _RelativeWidth() {
        Short align = Short.valueOf(com.sun.star.text.HoriOrientation.CENTER);
        try {
          oObj.setPropertyValue("HoriOrient",align);
        }
        catch (com.sun.star.lang.IllegalArgumentException ex) {}
        catch (com.sun.star.lang.WrappedTargetException ex) {}
        catch (com.sun.star.beans.PropertyVetoException ex) {}
        catch (com.sun.star.beans.UnknownPropertyException ex) {}

        testProperty("RelativeWidth");
    }

    /**
    * Forces environment recreation.
    */
    @Override
    protected void after() {
        disposeEnvironment();
    }

} // finish class _TextTable

