/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XAccessibleExtendedComponent.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-27 09:20:19 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
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

package complex.toolkit.interface_tests;

import lib.MultiMethodTest;
import com.sun.star.accessibility.XAccessibleExtendedComponent;
import com.sun.star.accessibility.XAccessibleStateSet;
import com.sun.star.accessibility.AccessibleStateType;
import com.sun.star.awt.XFont;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;
import share.LogWriter;

/**
 * Testing <code>com.sun.star.accessibility.XAccessibleExtendedComponent</code>
 * interface methods :
 * <ul>
 *  <li><code> getForeground()</code></li>
 *  <li><code> getBackground()</code></li>
 *  <li><code> getFont()</code></li>
 *  <li><code> isEnabled()</code></li>
 *  <li><code> getTitledBorderText()</code></li>
 *  <li><code> getToolTipText()</code></li>
 * </ul> <p>
 * @see com.sun.star.accessibility.XAccessibleExtendedComponent
 */
public class _XAccessibleExtendedComponent {

    private LogWriter log;
    private static final String className =
        "com.sun.star.accessibility.XAccessibleExtendedComponent" ;

    public XAccessibleExtendedComponent oObj = null;

    // temporary while accessibility package is in com.sun.star
    protected String getTestedClassName() {
        return className;
    }

    public _XAccessibleExtendedComponent(XInterface object, LogWriter log) {
        oObj = (XAccessibleExtendedComponent)UnoRuntime.queryInterface(
                                    XAccessibleExtendedComponent.class, object);
        this.log = log;
    }

    /**
     * Just calls the method.
     */
    public boolean _getFont() {
        XFont font = oObj.getFont();
        log.println("getFont(): " + font);
        return true;
    }

    /**
     * Calls the method and checks returned value.
     * Has OK status if returned value isn't null.
     */
    public boolean _getTitledBorderText() {
        String titleBorderText = oObj.getTitledBorderText();
        log.println("getTitledBorderText(): '" + titleBorderText + "'");
        return titleBorderText != null;
    }

    /**
     * Calls the method and checks returned value.
     * Has OK status if returned value isn't null.
     */
    public boolean _getToolTipText() {
        String toolTipText = oObj.getToolTipText();
        log.println("getToolTipText(): '" + toolTipText + "'");
        return toolTipText != null;
    }
}
