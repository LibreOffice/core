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
        Short align = new Short(com.sun.star.text.HoriOrientation.CENTER);
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
        Short align = new Short(com.sun.star.text.HoriOrientation.CENTER);
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
    protected void after() {
        disposeEnvironment();
    }

} // finish class _TextTable

