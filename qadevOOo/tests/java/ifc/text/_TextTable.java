/*************************************************************************
 *
 *  $RCSfile: _TextTable.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:14:39 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
        testProperty("PageDescName", "", "Standard");
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

